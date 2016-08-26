#pragma once
#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <cstdint>
#include <stdexcept>
#include <utility>

//Utility gives std::rel_ops which will fill in relational
//iterator operations so long as you provide the
//operators discussed in class.  In any case, ensure that
//all operations listed in this website are legal for your
//iterators:
//http://www.cplusplus.com/reference/iterator/RandomAccessIterator/
using namespace std::rel_ops;

#define minimum_capacity 8


namespace epl {
	class invalid_iterator {
	public:
		enum SeverityLevel { SEVERE, MODERATE, MILD, WARNING };
		SeverityLevel level;

		invalid_iterator(SeverityLevel level = SEVERE) { this->level = level; }
		virtual const char* what() const {
			switch (level) {
			case WARNING:   return "Warning"; // not used 
			case MILD:      return "Mild";
			case MODERATE:  return "Moderate";
			case SEVERE:    return "Severe";
			default:        return "ERROR"; // should not be used
			}
		}
	};
	template<typename T>
	class vector {
	public:
		T* data{ nullptr };
		uint64_t length{ 0 };
		uint64_t capacity{ 0 };
		uint64_t start{ 0 };
		uint64_t end1{ 0 };
		uint64_t version{ 0 };
		uint64_t reallocation_version{ 0 };
		uint64_t assignment_version{ 0 };

	public:
		vector(void) {
			data = (T*)::operator new(minimum_capacity*sizeof(T));
			length = 0;
			capacity = minimum_capacity;
			start = 0;
			end1 = 0;
		}
		explicit vector(uint64_t n) {
			if (n == 0) {
				data = (T*)::operator new(minimum_capacity*sizeof(T));
				length = 0;
				capacity = minimum_capacity;
				start = 0;
				end1 = 0;
			}
			else {
				data = (T*)::operator new(n*sizeof(T));
				for (uint64_t i = 0; i < n; i++) {
					new (data + i) T{};
					length = n;
					capacity = n;
					start = 1;
					end1 = n;
				}
			}
		}


		vector(const vector<T>& that) { copy(that); }

		void copy(const vector<T>& that) {
			length = that.length;
			capacity = that.capacity;
			start = that.start;
			end1 = that.end1;
			if (capacity == 0) {
				data = nullptr;
			}
			else {
				data = (T*)::operator new(capacity*sizeof(T));
				for (uint64_t i = 0; i < reallength(); i += 1) {
					new(data + (start - 1 + i))T{ that.operator[](i) };

				}
			}

		}

		vector(vector<T>&& tmp) {
			this->my_move(std::move(tmp));
			tmp.version += 1;
			tmp.reallocation_version += 1;
		}
		void my_move(vector<T>&& tmp) {
			this->data = tmp.data;
			this->length = tmp.length;
			this->capacity = tmp.capacity;
			this->start = tmp.start;
			this->end1 = tmp.end1;
			tmp.data = nullptr;
			tmp.length = 0;
			tmp.capacity = 0;
			tmp.start = 0;
			tmp.end1 = 0;
		}
		vector<T>& operator=(const vector<T>& rhs) {
			if (this != &rhs) {
				destroy();
				copy(rhs);
				reallocation_version += 1;
				assignment_version += 1;
				version += 1;
			}
			return *this;
		}
		vector<T>& operator=(vector<T>&& rhs) {
			if(this!=&rhs){
			std::swap(this->data, rhs.data);
			std::swap(this->length, rhs.length);
			std::swap(this->capacity, rhs.capacity);
			std::swap(this->start, rhs.start);
			std::swap(this->end1, rhs.end1);
			rhs.version += 1;
			rhs.reallocation_version += 1;
			this->assignment_version += 1;
			this->reallocation_version += 1;
			this->version += 1;
			}
			return *this;
		}

		void destroy(void) {
			for (uint64_t i = 0; i < reallength(); i++) {
				((*this).operator[](i)).~T();
			}
			operator delete(data);
		}
		~vector(void) { destroy(); }//length reallength

		uint64_t size(void)const {
			return reallength();
		}

		T& operator[](uint64_t k) {
			if (k >= reallength()) {
				throw std::out_of_range{ "subscript out of range" };
			}
			else { return data[(start - 1 + k) % capacity]; }
		}
		const T& operator[](uint64_t k)const {
			if (k >= reallength()) {
				throw std::out_of_range{ "subscript out of range" };
			}
			else { return data[(start - 1 + k) % capacity]; }
		}



		/*push_back push_front pop*/
		void doublespace(void) {
			capacity = capacity * 2;
		}

		uint64_t reallength(void) const {
			int64_t m;
			if (start == 0) {
				m = 0;
				return m;
			}
			else {
				m = end1 - start + 1;
				return m;
			}
}


		void push_back(const T& newvalue) {
			if (end1 == capacity) {
				doublespace();
				T*p = (T*)::operator new(capacity*sizeof(T));
				new(p + capacity / 4 + capacity / 2)T{ newvalue };
				for (uint64_t i = 0; i < reallength(); i++) {
					new(p + i + capacity / 4 + start - 1)T{ std::move(this->operator[](i)) };
				}

				for (uint64_t m = 0; m < reallength(); m++) {
					(this->operator[](m)).~T();
				}
				operator delete(data);
				start = start + capacity / 4;
				end1 = capacity / 4 + capacity / 2 + 1;
				length = reallength();
				data = p;
				p = nullptr;
				reallocation_version += 1;
				version += 1;

			}
			else {
				end1++;
				new(data + end1 - 1)T{ newvalue };
				length = reallength();
				if (start == 0) {
					start = 1;
					length = reallength();
				}
				version += 1;
			}
		}
		void push_back(T&& newvalue) {
			if (end1 == capacity) {
				doublespace();
				T*p = (T*)::operator new(capacity*sizeof(T));
				new(p + capacity / 4 + capacity / 2)T{ std::move(newvalue) };
				for (uint64_t i = 0; i < reallength(); i++) {
					new(p + i + capacity / 4 + start - 1)T{ std::move(this->operator[](i)) };
				}

				for (uint64_t m = 0; m < reallength(); m++) {
					(this->operator[](m)).~T();
				}
				operator delete(data);
				start = start + capacity / 4;
				end1 = capacity / 4 + capacity / 2 + 1;
				length = reallength();
				data = p;
				p = nullptr;
				reallocation_version += 1;
				version += 1;

			}
			else {
				end1++;
				new(data + end1 - 1)T{ std::move(newvalue) };
				length = reallength();
				if (start == 0) {
					start = 1;
					length = reallength();
				}
				version += 1;

			}
		}
		template<typename...Args>
		void emplace_back(Args&&... args) {
			if (end1 == capacity) {
				doublespace();
				T*p = (T*)::operator new(capacity*sizeof(T));
				new(p + capacity / 4 + capacity / 2)T{ std::forward<Args>(args)... };
				for (uint64_t i = 0; i < reallength(); i++) {
					new(p + i + capacity / 4 + start - 1)T{ std::move(this->operator[](i)) };
				}

				for (uint64_t m = 0; m < reallength(); m++) {
					(this->operator[](m)).~T();
				}
				operator delete(data);
				start = start + capacity / 4;
				end1 = capacity / 4 + capacity / 2 + 1;
				length = reallength();
				data = p;
				p = nullptr;
				reallocation_version += 1;
			}
			else {
				end1++;
				new(data + end1 - 1)T{ std::forward<Args>(args)... };
				length = reallength();
				if (start == 0) {
					start = 1;
					length = reallength();
				}
				version += 1;
			}
		}
		void push_front(const T& newvalue) {
			if (start == 1) {
				doublespace();
				T*p = (T*)::operator new(capacity*sizeof(T));
				new(p + capacity / 4 - 1)T{ newvalue };
				for (uint64_t i = 0; i < reallength(); i++) {
					new(p + i + capacity / 4)T{ std::move(this->operator[](i)) };
				}


				for (uint64_t m = 0; m < reallength(); m++) {
					(this->operator[](m)).~T();
				}
				operator delete(data);

				start = capacity / 4;
				end1 = end1 + capacity / 4;
				length = reallength();
				data = p;
				p = nullptr;
				reallocation_version += 1;
				version += 1;
			}

			else if (start == 0) {
				new(data)T{ newvalue };
				start = 1;
				end1 = 1;
				length = reallength();
				version += 1;
			}
			else {
				start--;
				new(data + start - 1)T{ newvalue };
				length = reallength();
				version += 1;
			}
		}
		void push_front(T&& newvalue) {
			if (start == 1) {
				doublespace();
				T*p = (T*)::operator new(capacity*sizeof(T));
				new(p + capacity / 4 - 1)T{ std::move(newvalue) };
				for (uint64_t i = 0; i < reallength(); i++) {
					new(p + i + capacity / 4)T{ std::move(this->operator[](i)) };
				}

				for (uint64_t m = 0; m < reallength(); m++) {
					(this->operator[](m)).~T();
				}
				operator delete(data);
				//delete[] data;
				start = capacity / 4;
				end1 = capacity / 4 + end1;
				length = reallength();
				data = p;
				p = nullptr;
				reallocation_version += 1;
				version += 1;
			}

			else if (start == 0) {
				new(data)T{ std::move(newvalue) };
				start = 1;
				end1 = 1;
				length = reallength();
				version += 1;
			}
			else {
				start--;
				new(data + start - 1)T{ std::move(newvalue) };
				length = reallength();
				version += 1;

			}
		}
		void pop_back(void) {
			if (reallength() == 0) {
				throw std::out_of_range("error");
			}
			else {
				data[end1 - 1].~T();
				if (start == end1) {
					start = 0;
					end1 = 0;
					length = reallength();

				}

				else {
					end1--;
					length = reallength();
				}
			}

			version += 1;
		}

		void pop_front(void) {
			if (reallength() == 0) {
				throw std::out_of_range("error");
			}


			else {
				if (start == end1) {
					data[start - 1].~T();
					start = 0;
					end1 = 0;
					length = reallength();
				}
				else {
					data[start - 1].~T();
					start++;
					length = reallength();
				}
			}

			version += 1;
		}
		class const_iterator : public std::iterator<std::random_access_iterator_tag, const T> {
		public:
			const T* ptr;
			const vector<T>* vec;
			uint64_t version;
			uint64_t index;
			uint64_t reallocation_version;
			uint64_t assignment_version;
		private:
			using Same = const_iterator;
			void invalid()const {
				bool is_modified = (version != vec->version);
				bool out_of_bound = (index > vec->size());
				if ((is_modified&&index<0) || (is_modified&&out_of_bound)) {
					throw invalid_iterator{ invalid_iterator::SEVERE };
				}
				else if ((reallocation_version != vec->reallocation_version) || assignment_version != vec->assignment_version) {
					throw invalid_iterator{ invalid_iterator::MODERATE };
				}
				else if (version != vec->version) {
					throw invalid_iterator{ invalid_iterator::MILD };
				}
			}
		public:
			const_iterator() {
					vec = nullptr;
					version = 0;
					index = 0;
					assignment_version = 0;
					reallocation_version = 0;

			}
			const_iterator(const vector<T>* x, uint64_t i) {
				version = x->version;
				vec = x;
				index = i;
				reallocation_version = x->reallocation_version;
				assignment_version = x->assignment_version;

			}
			const_iterator(const const_iterator& that) {
				that.invalid();
				vec = that.vec;
				version = that.version;
				index = that.index;
				assignment_version = that.assignment_version;
				reallocation_version = that.reallocation_version;


			}
			const_iterator& operator=(const const_iterator& that) {
				that.invalid();
				vec = that.vec;
				version = that.version;
				index = that.index;
				assignment_version = that.assignment_version;
				reallocation_version = that.reallocation_version;
				return *this;
			}
			const T& operator*(void) const {
				invalid();
				return vec->operator[](index);
			}
			const T& operator[](uint64_t k)const {
				invalid();
				return vec->operator[](index + k);
			}
			const T* operator->(void)const {
				invalid();
				return &(vec->operator[](index));
			}

			bool operator==(Same const& rhs)const {
				invalid();
				rhs.invalid();
				return this->index == rhs.index;
			}
			bool operator!=(Same const& rhs)const {
				invalid();
				rhs.invalid();
				return !(*this == rhs);
			}
			bool operator>(Same const& rhs)const {
				invalid();
				rhs.invalid();
				return(rhs.index  < this->index);
			}
			bool operator<(Same const& rhs)const {
				invalid();
				rhs.invalid();
				return(rhs.index > this->index);
			}
			bool operator<=(Same const&rhs)const {
				invalid();
				rhs.invalid();
				return !(rhs.index  < this->index);
			}
			bool operator>=(Same const&rhs)const {
				invalid();
				rhs.invalid();
				return !(this->index  < rhs.index);
			}
			Same& operator++(void) {
				invalid();
				++index;
				return *this;
			}
			Same& operator--(void) {
				invalid();
				--index;
				return*this;
			}
			Same operator++(int) {
				invalid();
				Same t{ *this };
				operator++();
				return t;
			}
			Same operator--(int) {
				invalid();
				Same t{ *this };
				operator--();
				return t;
			}
			Same operator+(uint64_t k) const {
				invalid();
				Same result{*this};
				result.index = this->index + k;
				return result;
			}
			Same& operator+=(uint64_t k) {
				invalid();
				index += k;
				return *this;
			}
			Same operator-(uint64_t k){
				invalid();
				Same result{*this};
				result.index = this->index - k;
				return result;
			}

			Same& operator-=(uint64_t k){
				invalid();
				index -= k;
				return *this;
			}
			uint64_t operator-(Same that){
				invalid();
				that.invalid();
				return this->index - that.index;
			}
			friend vector;
		};
		class iterator : public std::iterator<std::random_access_iterator_tag, T> {
		private:
			using Same = iterator;
			void invalid()const {
				bool is_modified = (version != vec->version);
				bool out_of_bound = (index > vec->size());
				if ((is_modified&&vec->size()<0) || (is_modified&&out_of_bound)) {
					throw invalid_iterator{ invalid_iterator::SEVERE };
				}
				else if ((reallocation_version != vec->reallocation_version) || assignment_version != vec->assignment_version) {
					throw invalid_iterator{ invalid_iterator::MODERATE };
				}
				else if (version != vec->version) {
					throw invalid_iterator{ invalid_iterator::MILD };
				}
			}
		public:
			T* ptr;
			vector<T>* vec;
			uint64_t version;
			uint64_t index;
			uint64_t reallocation_version;
			uint64_t assignment_version;
			iterator() {
				vec = nullptr;
				version = 0;
				index = 0;
				assignment_version = 0;
				reallocation_version = 0;
			}
			iterator(vector<T>* x, uint64_t i) {
				version = x->version;
				vec = x;
				index = i;
				reallocation_version = x->reallocation_version;
				assignment_version = x->assignment_version;
			}
			iterator(const iterator& that) {
				that.invalid();
				vec = that.vec;
				version = that.version;
				index = that.index;
				assignment_version = that.assignment_version;
				reallocation_version = that.reallocation_version;
			}
			iterator& operator=(const iterator& that) {
				that.invalid();
				vec = that.vec;
				version = that.version;
				index = that.index;
				assignment_version = that.assignment_version;
				reallocation_version = that.reallocation_version;
				return *this;
			}
		
			operator const_iterator() {
				return const_iterator(this->vec, this->index);
			}
			T& operator*(void){
				invalid();
				return vec->operator[](index);
			}
			T& operator[](uint64_t k) {
				invalid();
				return vec->operator[](index + k);
			}
		    T* operator->(void){
				invalid();
				return &(vec->operator[](index));
			}

			bool operator==(Same const& rhs)const {
				invalid();
				rhs.invalid();
				return this->index == rhs.index;
			}
			bool operator!=(Same const& rhs)const {
				invalid();
				rhs.invalid();
				return !(*this == rhs);
			}
			bool operator>(Same const& rhs)const {
				invalid();
				rhs.invalid();
				return(rhs.index  < this->index);
			}
			bool operator<(Same const& rhs)const {
				invalid();
				rhs.invalid();
				return(rhs.index >this->index);
			}
			bool operator<=(Same const&rhs)const {
				invalid();
				rhs.invalid();
				return !(rhs.index  < this->index);
			}
			bool operator>=(Same const&rhs)const {
				invalid();
				rhs.invalid();
				return !(this->index  < rhs.index);
			}
			Same& operator++(void) {
				invalid();
				++index;
				return *this;
			}
			Same& operator--(void) {
				invalid();
				--index;
				return*this;
			}
			Same operator++(int) {
				invalid();
				Same t{ *this };
				operator++();
				return t;
			}
			Same operator--(int) {
				invalid();
				Same t{ *this };
				operator--();
				return t;
			}
			Same operator+(uint64_t k){
				invalid();
				Same result{*this};
				result.index = this->index + k;
			    return result;
			}
			Same& operator+=(uint64_t k){
				invalid();
				index += k;
				return*this;
				
			}
			Same operator-(uint64_t k)const {
				invalid();
				Same result{*this};
				result.index = this->index - k;
				return result;
			}
			Same& operator-=(uint64_t k) {
				invalid();
				index -= k;
				return*this;
			}
			uint64_t operator-(Same that){
				invalid();
				that.invalid();
				return this->index - that.index;
			}
			friend vector;
};
public:

		iterator begin(void) {
			return iterator(this, 0);
		}
		iterator end(void) {
			return iterator(this, this->size());

		}
		const_iterator begin(void) const {
			return const_iterator(this, 0);
		}
		const_iterator end(void) const {
			return const_iterator(this, this->size());
		}
		
		template<typename It>
		void constructor(It b, It e, std::input_iterator_tag tag, T i) {
			data = (T*)::operator new(minimum_capacity*sizeof(T));
			length = 0;
			capacity = minimum_capacity;
			start = 0;
			end1 = 0;
			for (auto it = b; it != e; ++it) {
				push_back(*it);
			}
		}

		template<typename It>
		void constructor(It b, It e, std::random_access_iterator_tag, T i) {
			capacity = e - b;
			data = (T*)::operator new(capacity*sizeof(T));
			length = e - b;
			start = 1;
			end1 = e - b;
			for (uint64_t k = 0; k < reallength(); k += 1) {
				new(data + (start - 1 + k))T{ *(b + k) };
			}
		}
		template <typename It>
		vector(It b, It e) {
			typename std::iterator_traits<It>::iterator_category tag{};
			typename std::iterator_traits<It>::value_type val{};
			constructor(b, e, tag, val);
		}
		template <typename it>
		vector(const_iterator& b, const_iterator& e) {
			typename std::iterator_traits<it>::random_access_iterator_tag tag{};
			typename std::iterator_traits<it>::value_type val{};
			constructor(b, e, tag, val);
		}

		vector(std::initializer_list<T> list) :vector(list.begin(), list.end()) {}
};
}


#endif



