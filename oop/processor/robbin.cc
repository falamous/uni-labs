#include <functional>
#include <memory>
#include <vector>
#include <map>
#include <cstdint>
#include <iostream>


template<
class Key,
      class T,
      class Hash = std::hash<Key>,
      class KeyEqual = std::equal_to<Key>,
      class Allocator = std::allocator< std::pair<const Key, T> >
      > class my_map {
          using kv_pair_type = std::pair<Key, T>;


          class KeyList {
              std::shared_ptr<KeyList> next_;
              std::shared_ptr<KeyList> prev_;
              public:
              kv_pair_type value;

              KeyList(kv_pair_type kv_pair) :
                  value {kv_pair}
              { }
              std::shared_ptr<KeyList> push_back(kv_pair_type value) {
                  if (next_) {
                      return next_->push_back(value);
                  } else {
                      return next_ = std::shared_ptr<KeyList>(new KeyList(value));
                  }
              }
              ~KeyList() {
                  if (next_) {
                      next_->prev_ = prev_;
                  }
                  if (prev_) {
                      prev_->next_ = next_;
                  }
              }

          };

          struct iterator {
              using iterator_category = std::forward_iterator_tag;
              using difference_type   = std::ptrdiff_t;
              using value_type        = kv_pair_type;
              using pointer           = kv_pair_type*;  // or also kv_pair_type*
              using reference         = kv_pair_type&;  // or also kv_pair_type&
              iterator operator++() {
                  list_entry_ = list_entry_->next_;
              }
              iterator operator--() {
                  list_entry_ = list_entry_->prev_;
              }

              bool operator!=(const iterator& it) {
                  return list_entry_ != it.list_entry_;
              }
              bool operator==(const iterator& it) {
                  return list_entry_ != it.list_entry_;
              }

              kv_pair_type& operator*() {
                  return list_entry_->value;
              }
              iterator(std::shared_ptr<KeyList> list_entry) :
                  list_entry_ {list_entry}
              { }
              private:
              std::shared_ptr<KeyList> list_entry_;
          };

          static const uint8_t SLOT_FREE = 0;
          static const uint8_t SLOT_OCCUPIED = 2;
          static const uint8_t SLOT_UNOCCUPIED = 3;
          struct Slot {
              std::shared_ptr<KeyList> value;
              std::size_t hash;
              unsigned long psi;
              uint8_t occupation;

              Slot() {
                  clear();
              }

              void clear() {
                  psi = 0;
                  value = nullptr;
                  occupation = SLOT_FREE;
              }

              void unoccupy() {
                  value = nullptr;
                  occupation = SLOT_UNOCCUPIED;
              }

              bool is_free() {
                  return occupation == SLOT_FREE;
              }
              bool is_usable() {
                  return occupation != SLOT_OCCUPIED;
              }
              bool has_key(size_t key_hash, Key key) {
                  return occupation == SLOT_OCCUPIED && hash == key_hash && key == value->value.first;
              }
          };

          Hash hasher;
          size_t size_;
          size_t psi_sum_;
          std::vector<Slot> table_;

          std::shared_ptr<KeyList> key_list_start_;
          std::shared_ptr<KeyList> key_list_end_;

          public:

          void clear() {
              table_ = {{}};
              psi_sum_ = 0;
              size_ = 0;
              key_list_end_ = nullptr;
              key_list_start_ = nullptr;
          }
          my_map() { clear(); };
          ~my_map() {};

          std::size_t size() {
              return size_;
          }
          bool empty() {
              return size_ == 0;
          }

          iterator end() {
              return iterator(nullptr);
          }
          iterator begin() {
              return iterator(key_list_start_);
          }

          iterator find( const Key& key ) {

              auto hash = hasher(key);
              std::size_t avg_psi = psi_sum_ / (size_ + 1);
              long backward_index = (hash + avg_psi) % (table_.size());
              long forward_index = (hash + avg_psi) % (table_.size());



              while (!table_[backward_index].is_free() || !table_[forward_index].is_free()) {
                  if (!table_[backward_index].is_free()) {
                      if (table_[backward_index].has_key(hash, key)) {
                          return iterator(table_[backward_index].value);
                      }
                      backward_index = (backward_index - 1) % (long)table_.size();
                  }
                  if (!table_[forward_index].is_free()) {
                      if (table_[forward_index].has_key(hash, key)) {
                          return iterator(table_[forward_index].value);
                      }
                      forward_index = (forward_index + 1) % (long)table_.size();
                  }
                  if (forward_index == avg_psi || backward_index == avg_psi) {
                      break;
                  }
              }

              return iterator(nullptr);
          }

          std::pair<iterator,bool> insert( const kv_pair_type& value ) {
              auto find_res = find(value.first);
              if (find_res != end()) {
                  return std::make_pair(find_res, false);
              }
              return insert_or_assign(value);
          }

          std::pair<iterator,bool> insert_or_assign( kv_pair_type value ) {
              auto hash = hasher(value.first);
              long h = hash % table_.size();
              std::size_t avg_psi = psi_sum_ / (size_ + 1);

              size_t psi = 0;
              size_++;

              while (!table_[h].is_usable()) {
                  if (table_[h].has_key(hash, value.first)) {
                      table_[h].value->value = value;
                      size_--;
                      psi_sum_ -= psi;
                      return std::make_pair(iterator(table_[h].value), true);
                  }

                  if (psi > table_[h].psi) {
                      std::swap(table_[h].psi, psi);
                      std::swap(table_[h].value->value, value);
                      std::swap(table_[h].hash, hash);

                  }

                  h = (h + 1) % table_.size();
                  psi++;
                  psi_sum_++;

                  if (h == hash % table_.size()) {
                      rebuild(table_.size() * 2 + 1);
                      return insert_or_assign(value);
                  }
              }

              table_[h].occupation = SLOT_OCCUPIED;

              table_[h].psi = psi;
              table_[h].hash = hash;

              if (key_list_end_) {
                  key_list_end_ = key_list_end_->push_back(value);
              } else {
                  key_list_end_ = key_list_start_ = std::shared_ptr<KeyList>(new KeyList(value));
              }
              table_[h].value = key_list_end_;
              return std::make_pair(key_list_end_, true);
          }
          void rebuild(size_t entry_count=4096) {
              auto old_table = table_;
              clear();
              table_.resize(entry_count);
              for(auto entry: old_table) {
                  insert_or_assign(entry.value->value);
              }
          }

          void erase(Key key){
              size_t hash;
              size_t h;
              size_t i;
              size_t j;
              size_t k;

              hash = hasher(key);
              h = hash % table_.size();

              while (!table_[h].is_free()) {
                  if (table_[h].has_key(hash, key)) {
                      break;
                  }
                  h = (h + 1) % table_.size();
                  if (h == hash % table_.size()) {
                      return;
                  }
              }
              size_--;

              i = j = h;

              for(;;) {
                  table_[i].unoccupy();
                  j = (j + 1) % table_.size();
                  if (table_[j].is_usable() || table_[j].psi == 0) {
                      break;
                  }
                  k = table_[j].hash % table_.size();
                  if ((i<=j) ? ((i<k)&&(k<=j)) : ((i<k)||(k<=j))) {
                      continue;
                  }
                  table_[i] = table_[j];
                  i = j;
              }
          }

      };
