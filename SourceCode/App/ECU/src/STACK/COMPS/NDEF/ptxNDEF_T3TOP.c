hat data access with this operator is unchecked and
       *  out_of_range lookups are not defined. (For checked lookups
       *  see at().)
       */
      _GLIBCXX_NODISCARD
      reference
      operator[](size_type __n) _GLIBCXX_NOEXCEPT
      {
	__glibcxx_requires_subscript(__n);
	return this->_M_impl._M_start[difference_type(__n)];
      }

      /**
       *  @brief Subscript access to the data contained in the %deque.
       *  @param __n The index of the element for which data should be
       *  accessed.
       *  @return  Read-only (constant) reference to data.
       *
       *  This operator allows for easy, array-style, data access.
       *  Note that data access with this operator is unchecked and
       *  out_of_range lookups are not defined. (For checked lookups
       *  see at().)
       */
      _GLIBCXX_NODISCARD
      const_reference
      operator[](size_type __n) const _GLIBCXX_NOEXCEPT
      {
	__glibcxx_requires_subscript(__n);
	return this->_M_impl._M_start[difference_type(__n)];
      }

    protected:
      /// Safety check used only from at().
      void
      _M_range_check(size_type __n) const
      {
	if (__n >= this->size())
	  __throw_out_of_range_fmt(__N("deque::_M_range_check: __n "
				       "(which is %zu)>= this->size() "
				       "(which is %zu)"),
				   __n, this->size());
      }

    public:
      /**
       *  @brief  Provides access to the data contained in the %deque.
       *  @param __n The index of the element for which data should be
       *  accessed.
       *  @return  Read/write reference to data.
       *  @throw  std::out_of_range  If @a __n is an invalid index.
       *
       *  This function provides for safer data access.  The parameter
       *  is first checked that it is in the range of the deque.  The
       *  function throws out_of_range if the check fails.
       */
      reference
      at(size_type __n)
      {
	_M_range_check(__n);
	return (*this)[__n];
      }

      /**
       *  @brief  Provides access to the data contained in the %deque.
       *  @param __n The index of the element for which data should be
       *  accessed.
       *  @return  Read-only (constant) reference to data.
       *  @throw  std::out_of_range  If @a __n is an invalid index.
       *
       *  This function provides for safer data access.  The parameter is first
       *  checked that it is in the range of the deque.  The function throws
       *  out_of_range if the check fails.
       */
      const_reference
      at(size_type __n) const
      {
	_M_range_check(__n);
	return (*this)[__n];
      }

      /**
       *  Returns a read/write reference to the data at the first
       *  element of the %deque.
       */
      _GLIBCXX_NODISCARD
      reference
      front() _GLIBCXX_NOEXCEPT
      {
	__glibcxx_requires_nonempty();
	return *begin();
      }

      /**
       *  Returns a read-only (constant) reference to the data at the first
       *  element of the %deque.
       */
      _GLIBCXX_NODISCARD
      const_reference
      front() const _GLIBCXX_NOEXCEPT
      {
	__glibcxx_requires_nonempty();
	return *begin();
      }

      /**
       *  Returns a read/write reference to the data at the last element of the
       *  %deque.
       */
      _GLIBCXX_NODISCARD
      reference
      back() _GLIBCXX_NOEXCEPT
      {
	__glibcxx_requires_nonempty();
	iterator __tmp = end();
	--__tmp;
	return *__tmp;
      }

      /**
       *  Returns a read-only (constant) reference to the data at the last
       *  element of the %deque.
       */
      _GLIBCXX_NODISCARD
      const_reference
      back() const _GLIBCXX_NOEXCEPT
      {
	__glibcxx_requires_nonempty();
	const_iterator __tmp = end();
	--__tmp;
	return *__tmp;
      }

      // [23.2.1.2] modifiers
      /**
       *  @brief  Add data to the front of the %deque.
       *  @param  __x  Data to be added.
       *
       *  This is a typical stack operation.  The function creates an
       *  element at the front of the %deque and assigns the given
       *  data to it.  Due to the nature of a %deque this operation
       *  can be done in constant time.
       */
      void
      push_front(const value_type& __x)
      {
	if (this->_M_impl._M_start._M_cur != this->_M_impl._M_start._M_first)
	  {
	    _Alloc_traits::construct(this->_M_impl,
				     this->_M_impl._M_start._M_cur - 1,
				     __x);
	    --this->_M_impl._M_start._M_cur;
	  }
	else
	  _M_push_front_aux(__x);
      }

#if __cplusplus >= 201103L
      void
      push_front(value_type&& __x)
      { emplace_front(std::move(__x)); }

      template<typename... _Args>
#if __cplusplus > 201402L
	reference
#else
	void
#endif
	emplace_front(_Args&&... __args);
#endif

      /**
       *  @brief  Add data to the end of the %deque.
       *  @param  __x  Data to be added.
       *
       *  This is a typical stack operation.  The function creates an
       *  element at the end of the %deque and assigns the given data
       *  to it.  Due to the nature of a %deque this operation can be
       *  done in constant time.
       */
      void
      push_back(const value_type& __x)
      {
	if (this->_M_impl._M_finish._M_cur
	    != this->_M_impl._M_finish._M_last - 1)
	  {
	    _Alloc_traits::construct(this->_M_impl,
				     this->_M_impl._M_finish._M_cur, __x);
	    ++this->_M_impl._M_finish._M_cur;
	  }
	else
	  _M_push_back_aux(__x);
      }

#if __cplusplus >= 201103L
      void
      push_back(value_type&& __x)
      { emplace_back(std::move(__x)); }

      template<typename... _Args>
#if __cplusplus > 201402L
	reference
#else
	void
#endif
	emplace_back(_Args&&... __args);
#endif

      /**
       *  @brief  Removes first element.
       *
       *  This is a typical stack operation.  It shrinks the %deque by one.
       *
       *  Note that no data is returned, and if the first element's data is
       *  needed, it should be retrieved before pop_front() is called.
       */
      void
      pop_front() _GLIBCXX_NOEXCEPT
      {
	__glibcxx_requires_nonempty();
	if (this->_M_impl._M_start._M_cur
	    != this->_M_impl._M_start._M_last - 1)
	  {
	    _Alloc_traits::destroy(_M_get_Tp_allocator(),
				   this->_M_impl._M_start._M_cur);
	    ++this->_M_impl._M_start._M_cur;
	  }
	else
	  _M_pop_front_aux();
      }

      /**
       *  @brief  Removes last element.
       *
       *  This is a typical stack operation.  It shrinks the %deque by one.
       *
       *  Note that no data is returned, and if the last element's data is
       *  needed, it should be retrieved before pop_back() is called.
       */
      void
      pop_back() _GLIBCXX_NOEXCEPT
      {
	__glibcxx_requires_nonempty();
	if (this->_M_impl._M_finish._M_cur
	    != this->_M_impl._M_finish._M_first)
	  {
	    --this->_M_impl._M_finish._M_cur;
	    _Alloc_traits::destroy(_M_get_Tp_allocator(),
				   this->_M_impl._M_finish._M_cur);
	  }
	else
	  _M_pop_back_aux();
      }

#if __cplusplus >= 201103L
      /**
       *  @brief  Inserts an object in %deque before specified iterator.
       *  @param  __position  A const_iterator into the %deque.
       *  @param  __args  Arguments.
       *  @return  An iterator that points to the inserted data.
       *
       *  This function will insert an object of type T constructed
       *  with T(std::forward<Args>(args)...) before the specified location.
       */
      template<typename... _Args>
	iterator
	emplace(const_iterator __position, _Args&&... __args);

      /**
       *  @brief  Inserts given value into %deque before specified iterator.
       *  @param  __position  A const_iterator into the %deque.
       *  @param  __x  Data to be inserted.
       *  @return  An iterator that points to the inserted data.
       *
       *  This function will insert a copy of the given value before the
       *  specified location.
       */
      iterator
      insert(const_iterator __position, const value_type& __x);
#else
      /**
       *  @brief  Inserts given value into %deque before specified iterator.
       *  @param  __position  An iterator into the %deque.
       *  @param  __x  Data to be inserted.
       *  @return  An iterator that points to the inserted data.
       *
       *  This function will insert a copy of the given value before the
       *  specified location.
       */
      iterator
      insert(iterator __position, const value_type& __x);
#endif

#if __cplusplus >= 201103L
      /**
       *  @brief  Inserts given rvalue into %deque before specified iterator.
       *  @param  __position  A const_iterator into the %deque.
       *  @param  __x  Data to be inserted.
       *  @return  An iterator that points to the inserted data.
       *
       *  This function will insert a copy of the given rvalue before the
       *  specified location.
       */
      iterator
      insert(const_iterator __position, value_type&& __x)
      { return emplace(__position, std::move(__x)); }

      /**
       *  @brief  Inserts an initializer list into the %deque.
       *  @param  __p  An iterator into the %deque.
       *  @param  __l  An initializer_list.
       *  @return  An iterator that points to the inserted data.
       *
       *  This function will insert copies of the data in the
       *  initializer_list @a __l into the %deque before the location
       *  specified by @a __p.  This is known as <em>list insert</em>.
       */
      iterator
      insert(const_iterator __p, initializer_list<value_type> __l)
      {
	auto __offset = __p - cbegin();
	_M_range_insert_aux(__p._M_const_cast(), __l.begin(), __l.end(),
			    std::random_access_iterator_tag());
	return begin() + __offset;
      }

      /**
       *  @brief  Inserts a number of copies of given data into the %deque.
       *  @param  __position  A const_iterator into the %deque.
       *  @param  __n  Number of elements to be inserted.
       *  @param  __x  Data to be inserted.
       *  @return  An iterator that points to the inserted data.
       *
       *  This function will insert a specified number of copies of the given
       *  data before the location specified by @a __position.
       */
      iterator
      insert(const_iterator __position, size_type __n, const value_type& __x)
      {
	difference_type __offset = __position - cbegin();
	_M_fill_insert(__position._M_const_cast(), __n, __x);
	return begin() + __offset;
      }
#else
      /**
       *  @brief  Inserts a number of copies of given data into the %deque.
       *  @param  __position  An iterator into the %deque.
       *  @param  __n  Number of elements to be inserted.
       *  @param  __x  Data to be inserted.
       *
       *  This function will insert a specified number of copies of the given
       *  data before the location specified by @a __position.
       */
      void
      insert(iterator __position, size_type __n, const value_type& __x)
      { _M_fill_insert(__position, __n, __x); }
#endif

#if __cplusplus >= 201103L
      /**
       *  @brief  Inserts a range into the %deque.
       *  @param  __position  A const_iterator into the %deque.
       *  @param  __first  An input iterator.
       *  @param  __last   An input iterator.
       *  @return  An iterator that points to the inserted data.
       *
       *  This function will insert copies of the data in the range
       *  [__first,__last) into the %deque before the location specified
       *  by @a __position.  This is known as <em>range insert</em>.
       */
      template<typename _InputIterator,
	       typename = std::_RequireInputIter<_InputIterator>>
	iterator
	insert(const_iterator __position, _InputIterator __first,
	       _InputIterator __last)
	{
	  difference_type __offset = __position - cbegin();
	  _M_range_insert_aux(__position._M_const_cast(), __first, __last,
			      std::__iterator_category(__first));
	  return begin() + __offset;
	}
#else
      /**
       *  @brief  Inserts a range into the %deque.
       *  @param  __position  An iterator into the %deque.
       *  @param  __first  An input iterator.
       *  @param  __last   An input iterator.
       *
       *  This function will insert copies of the data in the range
       *  [__first,__last) into the %deque before the location specified
       *  by @a __position.  This is known as <em>range insert</em>.
       */
      template<typename _InputIterator>
	void
	insert(iterator __position, _InputIterator __first,
	       _InputIterator __last)
	{
	  // Check whether it's an integral type.  If so, it's not an iterator.
	  typedef typename std::__is_integer<_InputIterator>::__type _Integral;
	  _M_insert_dispatch(__position, __first, __last, _Integral());
	}
#endif

      /**
       *  @brief  Remove element at given position.
       *  @param  __position  Iterator pointing to element to be erased.
       *  @return  An iterator pointing to the next element (or end()).
       *
       *  This function will erase the element at the given position and thus
       *  shorten the %deque by one.
       *
       *  The user is cautioned that
       *  this function only erases the element, and that if the element is
       *  itself a pointer, the pointed-to memory is not touched in any way.
       *  Managing the pointer is the user's responsibility.
       */
      iterator
#if __cplusplus >= 201103L
      erase(const_iterator __position)
#else
      erase(iterator __position)
#endif
      { return _M_erase(__position._M_const_cast()); }

      /**
       *  @brief  Remove a range of elements.
       *  @param  __first  Iterator pointing to the first element to be erased.
       *  @param  __last  Iterator pointing to one past the last element to be
       *                erased.
       *  @return  An iterator pointing to the element pointed to by @a last
       *           prior to erasing (or end()).
       *
       *  This function will erase the elements in the range
       *  [__first,__last) and shorten the %deque accordingly.
       *
       *  The user is cautioned that
       *  this function only erases the elements, and that if the elements
       *  themselves are pointers, the pointed-to memory is not touched in any
       *  way.  Managing the pointer is the user's responsibility.
       */
      iterator
#if __cplusplus >= 201103L
      erase(const_iterator __first, const_iterator __last)
#else
      erase(iterator __first, iterator __last)
#endif
      { return _M_erase(__first._M_const_cast(), __last._M_const_cast()); }

      /**
       *  @brief  Swaps data with another %deque.
       *  @param  __x  A %deque of the same element and allocator types.
       *
       *  This exchanges the elements between two deques in constant time.
       *  (Four pointers, so it should be quite fast.)
       *  Note that the global std::swap() function is specialized such that
       *  std::swap(d1,d2) will feed to this function.
       *
       *  Whether the allocators are swapped depends on the allocator traits.
       */
      void
      swap(deque& __x) _GLIBCXX_NOEXCEPT
      {
#if __cplusplus >= 201103L
	__glibcxx_assert(_Alloc_traits::propagate_on_container_swap::value
			 || _M_get_Tp_allocator() == __x._M_get_Tp_allocator());
#endif
	_M_impl._M_swap_data(__x._M_impl);
	_Alloc_traits::_S_on_swap(_M_get_Tp_allocator(),
				  __x._M_get_Tp_allocator());
      }

      /**
       *  Erases all the elements.  Note that this function only erases the
       *  elements, and that if the elements themselves are pointers, the
       *  pointed-to memory is not touched in any way.  Managing the pointer is
       *  the user's responsibility.
       */
      void
      clear() _GLIBCXX_NOEXCEPT
      { _M_erase_at_end(begin()); }

    protected:
      // Internal constructor functions follow.

#if __cplusplus < 201103L
      // called by the range constructor to implement [23.1.1]/9

      // _GLIBCXX_RESOLVE_LIB_DEFECTS
      // 438. Ambiguity in the "do the right thing" clause
      template<typename _Integer>
	void
	_M_initialize_dispatch(_Integer __n, _Integer __x, __true_type)
	{
	  _M_initialize_map(_S_check_init_len(static_cast<size_type>(__n),
					      _M_get_Tp_allocator()));
	  _M_fill_initialize(__x);
	}

      // called by the range constructor to implement [23.1.1]/9
      template<typename _InputIterator>
	void
	_M_initialize_dispatch(_InputIterator __first, _InputIterator __last,
			       __false_type)
	{
	  _M_range_initialize(__first, __last,
			      std::__iterator_category(__first));
	}
#endif

      static size_t
      _S_check_init_len(size_t __n, const allocator_type& __a)
      {
	if (__n > _S_max_size(__a))
	  __throw_length_error(
	      __N("cannot create std::deque larger than max_size()"));
	return __n;
      }

      static size_type
      _S_max_size(const _Tp_alloc_type& __a) _GLIBCXX_NOEXCEPT
      {
	const size_t __diffmax = __gnu_cxx::__numeric_traits<ptrdiff_t>::__max;
	const size_t __allocmax = _Alloc_traits::max_size(__a);
	return (std::min)(__diffmax, __allocmax);
      }

      // called by the second initialize_dispatch above
      ///@{
      /**
       *  @brief Fills the deque with whatever is in [first,last).
       *  @param  __first  An input iterator.
       *  @param  __last  An input iterator.
       *  @return   Nothing.
       *
       *  If the iterators are actually forward iterators (or better), then the
       *  memory layout can be done all at once.  Else we move forward using
       *  push_back on each value from the iterator.
       */
      template<typename _InputIterator>
	void
	_M_range_initialize(_InputIterator __first, _InputIterator __last,
			    std::input_iterator_tag);

      // called by the second initialize_dispatch above
      template<typename _ForwardIterator>
	void
	_M_range_initialize(_ForwardIterator __first, _ForwardIterator __last,
			    std::forward_iterator_tag);
      ///@}

      /**
       *  @brief Fills the %deque with copies of value.
       *  @param  __value  Initial value.
       *  @return   Nothing.
       *  @pre _M_start and _M_finish have already been initialized,
       *  but none of the %deque's elements have yet been constructed.
       *
       *  This function is called only when the user provides an explicit size
       *  (with or without an explicit exemplar value).
       */
      void
      _M_fill_initialize(const value_type& __value);

#if __cplusplus >= 201103L
      // called by deque(n).
      void
      _M_default_initialize();
#endif

      // Internal assign functions follow.  The *_aux functions do the actual
      // assignment work for the range versions.

#if __cplusplus < 201103L
      // called by the range assign to implement [23.1.1]/9

      // _GLIBCXX_RESOLVE_LIB_DEFECTS
      // 438. Ambiguity in the "do the right thing" clause
      template<typename _Integer>
	void
	_M_assign_dispatch(_Integer __n, _Integer __val, __true_type)
	{ _M_fill_assign(__n, __val); }

      // called by the range assign to implement [23.1.1]/9
      template<typename _InputIterator>
	void
	_M_assign_dispatch(_InputIterator __first, _InputIterator __last,
			   __false_type)
	{ _M_assign_aux(__first, __last, std::__iterator_category(__first)); }
#endif

      // called by the second assign_dispatch above
      template<typename _InputIterator>
	void
	_M_assign_aux(_InputIterator __first, _InputIterator __last,
		      std::input_iterator_tag);

      // called by the second assign_dispatch above
      template<typename _ForwardIterator>
	void
	_M_assign_aux(_ForwardIterator __first, _ForwardIterator __last,
		      std::forward_iterator_tag)
	{
	  const size_type __len = std::distance(__first, __last);
	  if (__len > size())
	    {
	      _ForwardIterator __mid = __first;
	      std::advance(__mid, size());
	      std::copy(__first, __mid, begin());
	      _M_range_insert_aux(end(), __mid, __last,
				  std::__iterator_category(__first));
	    }
	  else
	    _M_erase_at_end(std::copy(__first, __last, begin()));
	}

      // Called by assign(n,t), and the range assign when it turns out
      // to be the same thing.
      void
      _M_fill_assign(size_type __n, const value_type& __val)
      {
	if (__n > size())
	  {
	    std::fill(begin(), end(), __val);
	    _M_fill_insert(end(), __n - size(), __val);
	  }
	else
	  {
	    _M_erase_at_end(begin() + difference_type(__n));
	    std::fill(begin(), end(), __val);
	  }
      }

      ///@{
      /// Helper functions for push_* and pop_*.
#if __cplusplus < 201103L
      void _M_push_back_aux(const value_type&);

      void _M_push_front_aux(const value_type&);
#else
      template<typename... _Args>
	void _M_push_back_aux(_Args&&... __args);

      template<typename... _Args>
	void _M_push_front_aux(_Args&&... __args);
#endif

      void _M_pop_back_aux();

      void _M_pop_front_aux();
      ///@}

      // Internal insert functions follow.  The *_aux functions do the actual
      // insertion work when all shortcuts fail.

#if __cplusplus < 201103L
      // called by the range insert to implement [23.1.1]/9

      // _GLIBCXX_RESOLVE_LIB_DEFECTS
      // 438. Ambiguity in the "do the right thing" clause
      template<typename _Integer>
	void
	_M_insert_dispatch(iterator __pos,
			   _Integer __n, _Integer __x, __true_type)
	{ _M_fill_insert(__pos, __n, __x); }

      // called by the range insert to implement [23.1.1]/9
      template<typename _InputIterator>
	void
	_M_insert_dispatch(iterator __pos,
			   _InputIterator __first, _InputIterator __last,
			   __false_type)
	{
	  _M_range_insert_aux(__pos, __first, __last,
			      std::__iterator_category(__first));
	}
#endif

      // called by the second insert_dispatch above
      template<typename _InputIterator>
	void
	_M_range_insert_aux(iterator __pos, _InputIterator __first,
			    _InputIterator __last, std::input_iterator_tag);

      // called by the second insert_dispatch above
      template<typename _ForwardIterator>
	void
	_M_range_insert_aux(iterator __pos, _ForwardIterator __first,
			    _ForwardIterator __last, std::forward_iterator_tag);

      // Called by insert(p,n,x), and the range insert when it turns out to be
      // the same thing.  Can use fill functions in optimal situations,
      // otherwise passes off to insert_aux(p,n,x).
      void
      _M_fill_insert(iterator __pos, size_type __n, const value_type& __x);

      // called by insert(p,x)
#if __cplusplus < 201103L
      iterator
      _M_insert_aux(iterator __pos, const value_type& __x);
#else
      template<typename... _Args>
	iterator
	_M_insert_aux(iterator __pos, _Args&&... __args);
#endif

      // called by insert(p,n,x) via fill_insert
      void
      _M_insert_aux(iterator __pos, size_type __n, const value_type& __x);

      // called by range_insert_aux for forward iterators
      template<typename _ForwardIterator>
	void
	_M_insert_aux(iterator __pos,
		      _ForwardIterator __first, _ForwardIterator __last,
		      size_type __n);


      // Internal erase functions follow.

      void
      _M_destroy_data_aux(iterator __first, iterator __last);

      // Called by ~deque().
      // NB: Doesn't deallocate the nodes.
      template<typename _Alloc1>
	void
	_M_destroy_data(iterator __first, iterator __last, const _Alloc1&)
	{ _M_destroy_data_aux(__first, __last); }

      void
      _M_destroy_data(iterator __first, iterator __last,
		      const std::allocator<_Tp>&)
      {
	if (!__has_trivial_destructor(value_type))
	  _M_destroy_data_aux(__first, __last);
      }

      // Called by erase(q1, q2).
      void
      _M_erase_at_begin(iterator __pos)
      {
	_M_destroy_data(begin(), __pos, _M_get_Tp_allocator());
	_M_destroy_nodes(this->_M_impl._M_start._M_node, __pos._M_node);
	this->_M_impl._M_start = __pos;
      }

      // Called by erase(q1, q2), resize(), clear(), _M_assign_aux,
      // _M_fill_assign, operator=.
      void
      _M_erase_at_end(iterator __pos)
      {
	_M_destroy_data(__pos, end(), _M_get_Tp_allocator());
	_M_destroy_nodes(__pos._M_node + 1,
			 this->_M_impl._M_finish._M_node + 1);
	this->_M_impl._M_finish = __pos;
      }

      iterator
      _M_erase(iterator __pos);

      iterator
      _M_erase(iterator __first, iterator __last);

#if __cplusplus >= 201103L
      // Called by resize(sz).
      void
      _M_default_append(size_type __n);

      bool
      _M_shrink_to_fit();
#endif

      ///@{
      /// Memory-handling helpers for the previous internal insert functions.
      iterator
      _M_reserve_elements_at_front(size_type __n)
      {
	const size_type __vacancies = this->_M_impl._M_start._M_cur
				      - this->_M_impl._M_start._M_first;
	if (__n > __vacancies)
	  _M_new_elements_at_front(__n - __vacancies);
	return this->_M_impl._M_start - difference_type(__n);
      }

      iterator
      _M_reserve_elements_at_back(size_type __n)
      {
	const size_type __vacancies = (this->_M_impl._M_finish._M_last
				       - this->_M_impl._M_finish._M_cur) - 1;
	if (__n > __vacancies)
	  _M_new_elements_at_back(__n - __vacancies);
	return this->_M_impl._M_finish + difference_type(__n);
      }

      void
      _M_new_elements_at_front(size_type __new_elements);

      void
      _M_new_elements_at_back(size_type __new_elements);
      ///@}


      ///@{
      /**
       *  @brief Memory-handling helpers for the major %map.
       *
       *  Makes sure the _M_map has space for new nodes.  Does not
       *  actually add the nodes.  Can invalidate _M_map pointers.
       *  (And consequently, %deque iterators.)
       */
      void
      _M_reserve_map_at_back(size_type __nodes_to_add = 1)
      {
	if (__nodes_to_add + 1 > this->_M_impl._M_map_size
	    - (this->_M_impl._M_finish._M_node - this->_M_impl._M_map))
	  _M_reallocate_map(__nodes_to_add, false);
      }

      void
      _M_reserve_map_at_front(size_type __nodes_to_add = 1)
      {
	if (__nodes_to_add > size_type(this->_M_impl._M_start._M_node
				       - this->_M_impl._M_map))
	  _M_reallocate_map(__nodes_to_add, true);
      }

      void
      _M_reallocate_map(size_type __nodes_to_add, bool __add_at_front);
      ///@}

#if __cplusplus >= 201103L
      // Constant-time, nothrow move assignment when source object's memory
      // can be moved because the allocators are equal.
      void
      _M_move_assign1(deque&& __x, /* always equal: */ true_type) noexcept
      {
	this->_M_impl._M_swap_data(__x._M_impl);
	__x.clear();
	std::__alloc_on_move(_M_get_Tp_allocator(), __x._M_get_Tp_allocator());
      }

      // When the allocators are not equal the operation could throw, because
      // we might need to allocate a new map for __x after moving from it
      // or we might need to allocate new elements for *this.
      void
      _M_move_assign1(deque&& __x, /* always equal: */ false_type)
      {
	if (_M_get_Tp_allocator() == __x._M_get_Tp_allocator())
	  return _M_move_assign1(std::move(__x), true_type());

	constexpr bool __move_storage =
	  _Alloc_traits::_S_propagate_on_move_assign();
	_M_move_assign2(std::move(__x), __bool_constant<__move_storage>());
      }

      // Destroy all elements and deallocate all memory, then replace
      // with elements created from __args.
      template<typename... _Args>
      void
      _M_replace_map(_Args&&... __args)
      {
	// Create new data first, so if allocation fails there are no effects.
	deque __newobj(std::forward<_Args>(__args)...);
	// Free existing storage using existing allocator.
	clear();
	_M_deallocate_node(*begin()._M_node); // one node left after clear()
	_M_deallocate_map(this->_M_impl._M_map, this->_M_impl._M_map_size);
	this->_M_impl._M_map = nullptr;
	this->_M_impl._M_map_size = 0;
	// Take ownership of replacement memory.
	this->_M_impl._M_swap_data(__newobj._M_impl);
      }

      // Do move assignment when the allocator propagates.
      void
      _M_move_assign2(deque&& __x, /* propagate: */ true_type)
      {
	// Make a copy of the original allocator state.
	auto __alloc = __x._M_get_Tp_allocator();
	// The allocator propagates so storage can be moved from __x,
	// leaving __x in a valid empty state with a moved-from allocator.
	_M_replace_map(std::move(__x));
	// Move the corresponding allocator state too.
	_M_get_Tp_allocator() = std::move(__alloc);
      }

      // Do move assignment when it may not be possible to move source
      // object's memory, resulting in a linear-time operation.
      void
      _M_move_assign2(deque&& __x, /* propagate: */ false_type)
      {
	if (__x._M_get_Tp_allocator() == this->_M_get_Tp_allocator())
	  {
	    // The allocators are equal so storage can be moved from __x,
	    // leaving __x in a valid empty state with its current allocator.
	    _M_replace_map(std::move(__x), __x.get_allocator());
	  }
	else
	  {
	    // The rvalue's allocator cannot be moved and is not equal,
	    // so we need to individually move each element.
	    _M_assign_aux(std::make_move_iterator(__x.begin()),
			  std::make_move_iterator(__x.end()),
			  std::random_access_iterator_tag());
	    __x.clear();
	  }
      }
#endif
    };

#if __cpp_deduction_guides >= 201606
  template<typename _InputIterator, typename _ValT
	     = typename iterator_traits<_InputIterator>::value_type,
	   typename _Allocator = allocator<_ValT>,
	   typename = _RequireInputIter<_InputIterator>,
	   typename = _RequireAllocator<_Allocator>>
    deque(_InputIterator, _InputIterator, _Allocator = _Allocator())
      -> deque<_ValT, _Allocator>;
#endif

  /**
   *  @brief  Deque equality comparison.
   *  @param  __x  A %deque.
   *  @param  __y  A %deque of the same type as @a __x.
   *  @return  True iff the size and elements of the deques are equal.
   *
   *  This is an equivalence relation.  It is linear in the size of the
   *  deques.  Deques are considered equivalent if their sizes are equal,
   *  and if corresponding elements compare equal.
  */
  template<typename _Tp, typename _Alloc>
    _GLIBCXX_NODISCARD
    inline bool
    operator==(const deque<_Tp, _Alloc>& __x, const deque<_Tp, _Alloc>& __y)
    { return __x.size() == __y.size()
	     && std::equal(__x.begin(), __x.end(), __y.begin()); }

#if __cpp_lib_three_way_comparison
  /**
   *  @brief  Deque ordering relation.
   *  @param  __x  A `deque`.
   *  @param  __y  A `deque` of the same type as `__x`.
   *  @return  A value indicating whether `__x` is less than, equal to,
   *           greater than, or incomparable with `__y`.
   *
   *  See `std::lexicographical_compare_three_way()` for how the determination
   *  is made. This operator is used to synthesize relational operators like
   *  `<` and `>=` etc.
  */
  template<typename _Tp, typename _Alloc>
    [[nodiscard]]
    inline __detail::__synth3way_t<_Tp>
    operator<=>(const deque<_Tp, _Alloc>& __x, const deque<_Tp, _Alloc>& __y)
    {
      return std::lexicographical_compare_three_way(__x.begin(), __x.end(),
						    __y.begin(), __y.end(),
						    __detail::__synth3way);
    }
#else
  /**
   *  @brief  Deque ordering relation.
   *  @param  __x  A %deque.
   *  @param  __y  A %deque of the same type as @a __x.
   *  @return  True iff @a x is lexicographically less than @a __y.
   *
   *  This is a total ordering relation.  It is linear in the size of the
   *  deques.  The elements must be comparable with @c <.
   *
   *  See std::lexicographical_compare() for how the determination is made.
  */
  template<typename _Tp, typename _Alloc>
    _GLIBCXX_NODISCARD
    inline bool
    operator<(const deque<_Tp, _Alloc>& __x, const deque<_Tp, _Alloc>& __y)
    { return std::lexicographical_compare(__x.begin(), __x.end(),
					  __y.begin(), __y.end()); }

  /// Based on operator==
  template<typename _Tp, typename _Alloc>
    _GLIBCXX_NODISCARD
    inline bool
    operator!=(const deque<_Tp, _Alloc>& __x, const deque<_Tp, _Alloc>& __y)
    { return !(__x == __y); }

  /// Based on operator<
  template<typename _Tp, typename _Alloc>
    _GLIBCXX_NODISCARD
    inline bool
    operator>(const deque<_Tp, _Alloc>& __x, const deque<_Tp, _Alloc>& __y)
    { return __y < __x; }

  /// Based on operator<
  template<typename _Tp, typename _Alloc>
    _GLIBCXX_NODISCARD
    inline bool
    operator<=(const deque<_Tp, _Alloc>& __x, const deque<_Tp, _Alloc>& __y)
    { return !(__y < __x); }

  /// Based on operator<
  template<typename _Tp, typename _Alloc>
    _GLIBCXX_NODISCARD
    inline bool
    operator>=(const deque<_Tp, _Alloc>& __x, const deque<_Tp, _Alloc>& __y)
    { return !(__x < __y); }
#endif // three-way comparison

  /// See std::deque::swap().
  template<typename _Tp, typename _Alloc>
    inline void
    swap(deque<_Tp,_Alloc>& __x, deque<_Tp,_Alloc>& __y)
    _GLIBCXX_NOEXCEPT_IF(noexcept(__x.swap(__y)))
    { __x.swap(__y); }

#undef _GLIBCXX_DEQUE_BUF_SIZE

_GLIBCXX_END_NAMESPACE_CONTAINER

#if __cplusplus >= 201103L
  // std::allocator is safe, but it is not the only allocator
  // for which this is valid.
  template<class _Tp>
    struct __is_bitwise_relocatable<_GLIBCXX_STD_C::deque<_Tp>>
    : true_type { };
#endif

_GLIBCXX_END_NAMESPACE_VERSION
} // namespace std

#endif /* _STL_DEQUE_H */
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      MZx                   @                                   x   º ´	Í!¸LÍ!This program cannot be run in DOS mode.$  PE  L /b¬`        à "!  ô   ’      P5                                    Ü  @A                   Çk ò   ¹l (    Ğ ˆ           Œ °'   à P  4h                    ¤f    ` ¼           n 8  ài                     .text   ùò      ô                    `.rdata  ôj     l   ø              @  @.data   ä   €  
   d             @  À.00cfg            n             @  @.tls    	    °     p             @  À.voltbl H    À     r                 .rsrc   ˆ   Ğ     t             @  @.reloc  P   à     z             @  B                                                                                                                                                                                                                                                                                                                                                U‰å1À@]Â ÌÌU‰åV‰Îh¤aèx  ‰ñh_èl  ÿvÿ€nƒøÿ•À^]ÃU‰åSWV‹u‹]‰ñh  èî  ÿvÿvSÿŒn‰Ç;Fu&ÿˆnƒøzu‰ñh   èÃ  ÿvÿvSÿŒn‰Ç…ÿt;~s‰ñWèÑ  1Àëÿˆn‰Á·Á  €…ÉNÁ^_[]ÃU‰åSWVƒì‰Ö‰Ï¡€1è‰Eğ‰Ñh  èb  ‰ñWèÜ  ƒ>r1‹F·Hfƒù:ufƒx\u· ƒàßƒÀ¿fƒøsësfƒ8\ufƒù\tg1ÀH]ä‰‰C‰C‰Ùè§  Sj èÿÿÿƒÄ…Àxd‰ñÿuäÿuìèŸ  j\ÿuìèƒ4  ƒÄ…ÀtJ]ä‹S)ĞƒÀÑø‰ñPRèx  ‰ñWè  ‰Ùèg  ‰ñè‚şÿÿ1É„À¾ €Eñ‹Mğ1éè  ‰ğë‰Ãë»@ €Mäè5  ‹Mğ1éèı  ‰ØƒÄ^_[]ÃÌÌÌÌÌÌÌU‰åSWVƒäğƒìP¡€1è‰D$Hƒ} „×   ‹E‹M‹U|$8ÇG    ‰‰O‰GfvÀt$fFfÇF ÿÿÿÿ‰ñè¬  L$è£  L$(èš  VWè£  ƒÄh j$èà  ƒÄ…Àtt‰Ã‹t$8‹|$<‹L$@‹T$DÇ@   Ç ğ^¡Ø“…Àt‰L$‹P‰|$‰÷‰ÖÿQ‰ò‰ş‹|$‹L$ÇĞ^‰s‰{‰K‰S‹M‰K‹QÿPÇC    ë	¾@ €ëB1Û‹L$8‹T$<Sÿt$Hÿt$HèA   ƒÄ‰Æ…Ût‹SÿPL$(èë  L$èâ  L$èÙ  ‹L$H1éè   ‰ğeô^_[]Â U‰åSWVƒì‰Ö‰Ï¡€1è‰Eğ1ÀHMä‰‰A‰Aè†  …ÿtfƒ? tUä‰ùèUıÿÿ‰Ç1ÛCë‹M1ÛUäSSèË  ƒÄ‰Ç…ÿuÿuÿuVSjÿuìè  ƒÄ‰ÇMäèF  ‹Mğ1éè  ‰øƒÄ^_[]ÃÌÌÌÌÌÌÌÌU‰åSWVƒäğì   ¡€1è‰„$ˆ   ƒ} „@  ‹E1ÉT$`‰J‰J‰J‰‰Ó1ÿOt$0‰~ fvÀfFf‰ñè½  L$<è´  L$Hè«  VSè´	  ƒÄL$$‰9‰y‰yè  \$‰;‰{‰{‰Ùè}  t$‰>‰~‰~‰ñèj  D$`‹…Ét fƒ9 tT$$è4üÿÿ…À„·   ‰Ã1À‹}ëU‹L$lT$$VSè   ƒÄ‰Ã‹}1À…Ûu8ƒ|$ tt$‰ñhøaèÈ  ‰ñÿt$ÿt$èã  1Ûÿt$ÿt$$è:  ƒÄ‰L$èò  L$èé  L$$èà  L$Hè×  L$<èÎ  L$0èÅ  ë»@ €‹Œ$ˆ   1éè‚  ‰Øeô^_[]Â ‹t$,„$„   Ç     PVèÛì  …À„¦   ‰Ã‰t$t$x‰>‰~‰~‰ñèY  ‰ØÑè@‰ñPè´  „ÀtZÿ´$€   Sÿ´$Œ   ÿt$è—ì  …Àt>1ÀL$t‰T$p‰RQh¨aÿ´$Œ   èxì  …À‹}tQ‹D$t…ÀtIL$PèÜ  1ÛëOÿˆn·ØË  €…ÀNØL$xèã  é şÿÿÿˆn·ØË  €…ÀNØé‡şÿÿÿˆn·ØË  €…ÀNØL$xè¨  éhşÿÿU‰åSWVƒäğƒì`‰T$‰L$¡€1è‰D$Xt$1À¿   )Çƒ|$‰D$Eø1ÀH‰D$‰D$ ‰D$$‰ñèC  ‰|$‹<½ğ`WèÃB  ƒÄ‰ÃƒÀ*‰ñPèŠ  ‰ñj*haè(  ‰ñS\$4W‹uèê  ‹L$$1Òÿt$Vèà  ƒÄ„À…+  ‹L$$1ÒBÿt$VèÄ  ƒÄ„À…  ¡à“‹¼‰d‹,   ‹Š;   ¦  ¡Ü“…ÀtsÇD$,ÿÿÿÿ1É‰L$(T$,RQL$0QjÿĞƒøzuQƒ|$( tJ1ÀH‰D$0‰D$4‰D$8‰Ùèd  ‹D$(Ñè@‰ÙPè½  „ÀtD$,Pÿt$<D$0PjÿÜ“…Àt%‰ÙèB  t$‰ñè7  ‹D$@ƒø…«şÿÿéx  ƒ|$, tÔ‹D$‹…Ìc‰D$‹t$8ƒÆ"1ÿÿt$ÿvîè?  ƒÄ…ÀtGƒÆ4;|$,rãë·‰D$‰D$H·Fş‰D$L·Fü‰D$P·Fú‰D$T‹|$‰ùÿvæè  ‰Ùè®
  L$H‰úèŠ  ‹}…ÿtz„Àtv‰ùjèå
  1ÀH‰D$B‰D$>fvÀfD$0j
jSÿt$èG=  ƒÄ…À…ÿÿÿ‰ùSè1  ¾   j
jSÿt$è!=  ƒÄ…À…ùşÿÿ‰ùhFdèÙ  ‰ùSèÑ  NuĞëR„À„ØşÿÿëHhà“èë  ƒÄƒ=à“ÿ…@şÿÿh`dÿ”nhJdPÿœn£Ü“hà“è  ƒÄéşÿÿ‹M…Ét‹D$ÿ4…aè‘
  L$è°	  1öë¾ €‹L$X1éèn  ‰ğeô^_[]ÃÌÌÌÌÌÌÌÌU‰åSWVƒäğƒì0‹]¡€1è‰D$(…Ût\‹M¾W €…ÉtU‹}…ÿtNfvÀT$ff$èW   ¾W €„Àt/‰â‰ùèE   „Àt"1É1À@1ÒJ‹4Œ9tŒw*r&Aƒùuï1Àë¾@ €‹L$(1éèÒ  ‰ğeô^_[]Â ‰Ğ‰1öëáU‰åSWVƒì‰×‰Mä¡€1è‰EğEìÇ     ³1ö±öÁu	Ç·    ëE‰Màˆ]ëj
EìP‹]äSèB  ƒÄ‰·‹Eì9Øt…Àtfƒ8.uƒÀ‰Eä‹Màë1É…öŠ]ëu;EätƒşF’Ã‰ÆƒøuöÓ€ã‹Mğ1éè%  ‰ØƒÄ^_[]ÃÌÌÌÌÌÌÌÌÌÌÌÌÌÌÌU‰å1ÀÿuPPPè÷ÿÿ]Â U‰åSWVƒäğì0  ‰Ö‰Ï¡€1è‰„$(  D$ h  hÿ   Pè‡)  ƒÄÇD$  D$Ç ÿÿÿÿ¹  €)ñPh j WQÿ<‰1Û…À…Š   D$PD$$PSSh_ÿt$ÿ@‰‰Æÿt$ÿ4‰»    …öuZ‹u‰ñD$ PèT  j\ÿvèb*  ƒÄ…Àt8‰ÇƒÇfvÀT$f‰ùègşÿÿ„Àt‹M…ÉtWè  L$‰òè   ‰Ãë1Û‹Œ$(  1éèõ  ‰Øeô^_[]ÃU‰åWV‰Ö1À‹àc9w
rƒÀƒøuë‰ñ^_]éôÿÿ‹=ønhğcÿ×ÿvÿ×hBdÿ×1À^_]ÃÌÌÌÌÌÌÌÌU‰åWV‹U‹u‹MÇ    ‹>¸@ €…ÿt%ÿ‰3ŠNu?~ØÉÒKu6~¶µOu-~îlÁMtë"ƒ~ u~À   u~   Fu
‰
‹QÿP1À^_]Â ÌÌÌÌU‰å‹M1À@ğÁA@]Â ÌÌÌÌÌÌÌÌÌÌÌÌÌU‰åV‹M1öNğÁqNu…Ét‹jÿP¡Ø“…Àt‹PÿQ‰ğ^]Â ÌÌÌÌÌÌÌÌÌÌÌÌÌU‰åWVƒì‹E‹u‹€1é‰Mô…Àx ‹MÇEğ    …ÉtD‹}ğWh€dQÿ‹ë4‹N …É~ÙI‰N ‹N‹VVÿvÿvèŞõÿÿƒÄ…Ày/‹N‹j PQÿRë!1É‹V‹2QPRÿV‹Eğ…ÀtÇEğ    ‹PÿQ‹Mô1éè2  1ÀƒÄ^_]Â ÌÌÌÌÌÌÌÌÌÌÌU‰åWV‰Î‹}ÇĞ^‹A…ÀtÇF    ‹PÿQÇF  À…ÿt	Vè´  ƒÄ‰ğ^_]Â U‰åÌÌÌU‰åSWVƒì‹}‹u¡€1è‰Eğ1Û¹üdºÌdjSVWèn   ƒÄƒÆG¹`eºBeSSVPèR   ƒÄEì‰1ÀHuà‰‰F‰F‰ñèˆ  ƒÇ¹Æeº”eSWVEìPè   ƒÄ‰ñèw  ‹Mğ1éè?
  ƒÄ^_[]ÃU‰åSWVƒì‰UìŠ]‹E‰Eğ‹u‹}„ÛtÆ`ˆVQè¶  ƒÄ„Àt'‹F‰ƒ}ğ tlj
j Pè•=  ƒÄ1Éƒø”Á‹Eğ‰ëP„Ûu	€=`ˆ tC¹  €èU   „Àt	Æ`ˆë¹  €è>   ¢`ˆ„Àt¹  €‹UìÿuğVWès   ƒÄ„ÀtƒÄ^_[]Ã¹  €‹UìƒÄ^_[]éP   U‰åSWVƒì¡€1è‰Eğ1À}ì‰Wh  PhfQÿ<‰‰Æÿ7ÿ4‰…ö”Ã‹Mğ1éè*	  ‰ØƒÄ^_[]ÃU‰åSWVƒì4‰UÀ‰MÄ¡€1è‰Eğ1ÿOMØ‰9‰y‰yè  h²dÿ”nhdPÿœn…Àt$‰ÃuØ‰ñhƒ   èL  ‹F‹NUä‰QRÿÓ…Àt4EäÇ     PÿL‰…Àx-MØÿuäèœ  „Àuÿˆnÿuäÿ\‰ë‹EäHMØPè&  MÌ‰9‰y‰yè€  uä‰>‰~‰~‰ñèn  Vj èÈïÿÿƒÄ…Àx"j\ÿuìèW%  ƒÄ…Àu‹EìƒÀMÌPè)  ëMÌè³  Mäè?  ÇEÈ    ‹EÀ…Àt\fƒ8 tV]ä‰;‰{‰{P‰Æè—8  ƒÄƒÀ*‰ÙPè>  ‰Ùhfè¨  ‰ÙVè   EÈPjj ÿsÿuÄÿ<‰‰Æ‰ÙèÙ  …öt&1ÛMÌèË  MØèÃ  ‹Mğ1éè‹  ‰ØƒÄ4^_[]Ã‹}‹u‹Uà‹MÈÿuWVèB   ƒÄ³„Àu.‹UÔ‹MÈÿuWVè)   ƒÄ„Àu‹MÈºdfÿuWVè   ƒÄ‰ÃÿuÈÿ4‰ë„U‰åSWVƒì‰Ö‰Ï‹]‹M‹E‰Eà¡€1è‰Eğ…ÛtL‰Mä1ÀMì‰UèÇ   RQPjVPWÿ8‰…ÀtA‰ù‰òÿuä‹uàVèX   ƒÄ„Àt;j
j ÿ6èb:  ƒÄë‹Mğ1éè¬  ‰ù‰òƒÄ^_[]é'   ‹Eì1Éƒø”Á‰³ë1Û‹Mğ1éè  ‰ØƒÄ^_[]ÃU‰åSWVì  ‰Ö‰àıÿÿ¡€1è‰Eğ1Û½èıÿÿh  SWè"  ƒÄ…äıÿÿÇ   PWSjVSÿµàıÿÿÿ8‰‰Æ…Àu‹}‹]‰ù…èıÿÿPè  ‹G‰…ö”Ã‹Mğ1éèó  ‰ØÄ  ^_[]ÃÌÌU‰å‰È1É‰‰H‰H]ÃÌU‰åVÇ    ‹A…Àt‰ÎfÇ   ‹A…ÀtPèˆ  ƒÄ1À‰F‰F^]ÃÌU‰åV‰Î‹E1É‰‰N‰N‰ñPè   ‰ğ^]Â U‰åSWVP‹]°9Ysfƒûÿt=‰ÎC1ÉIÀBÁPè%  ƒÄ‰Ç…Àt@‹…Àt‹N‰MğÀƒÀPQWè¥Œ  ‹EğƒÄë1Àë fÇ  ‹F…Àt	Pèî  ƒÄ‰~‰^…ÿ•ÀƒÄ^_[]Â ÌU‰åWV‰Î‹}…ÿtWè“5  ƒÄë1À‰ñPWè   ^_]Â U‰åSWV‰Î‹]Ç    ‹A…ÀtfÇ   …Ût<‹}‰ñWè(ÿÿÿ„Àt1?PSÿvèŒ  ƒÄ‹F9ør‰>‹N…ÉtfÇy  9ø“Àë°ë1À^_[]Â U‰åÇ    ‹A…ÀtfÇ   ]ÃÌU‰å‹E‹Q9Âr‰‹I…ÉtfÇA  9Â“À]Â ÌU‰åWV‰Î‹}…ÿtWèÁ4  ƒÄë1À‰ñPWè   ^_]Â U‰åSWVƒ} tN‰Î‹]‹9ßr?‰ñWècşÿÿ„Àt3‹ÀFÛSÿuPè<‹  ƒÄ‹F9ør‰>‹N…ÉtfÇy  9ø“Àë1Àë°^_[]Â ÌÌÌU‰åSWV‹E…À¿ZaEø‹E ƒÃ1ö9ÃvSÿX‰‰Æ…ÀtSWVèÕŠ  ƒÄ‰ğ^_[]ÃÌÌÌU‰åSWV‹]1ÀPPSÿ|n…Àt&‰Ç‹u‰ñPè³ıÿÿ„ÀtWÿvSÿ|n‰ñPèÊşÿÿë1À^_[]ÃÌÌÌU‰åWVÿuÿğn…Àt+‰ÆhhfPÿœn…Àt/ŠM¶ÉÿuÿuÿuÿuQÿĞ‰Çë*ÿˆn·øÏ  €…ÀNøë+ÿˆn·øÏ  €…ÀNøh”fVÿœn…ÀtVÿPn‰ø^_]ÃU‹ìjÿhÿd¡    PQSVW¡€3ÅPEôd£    ‰eğÿuƒeü è#  Yë¸r& Ã3À‹Môd‰    Y_^[ÉÃÌÌÌÌÌéy2  U‹ì]éõ  éíÿÿÿÌÌÌÌèB  j è§  Y„Àthp( è  Y3ÀÃjè)  ÌU‹ìVW¿p‰Wÿ4n‹uƒ> uƒÿë)jdèw   Yëìƒ>ÿtñd¡,   ‹¼‰‹ˆ¡ €‰   Wÿän_^]ÃU‹ìV¾p‰Vÿ4n‹ €‹EA‰ €V‰d¡,   ‹¼‰‹ˆ¡ €‰   ÿän^]éN   U‹ìV‹5ˆ‰…ötÿu‹Îhp‰hh‰ÿ  ÿÖë$¾p‰Vÿänj ÿuÿ5l‰ÿ@oVÿ4n^]ÃV‹5Œ‰…öthh‰‹Îÿ  ÿÖ^Ãÿ5l‰ÿoÿ5l‰ÿo^ÃVWh   hp‰ÿÈnhÿ”n‹ğ…öuhLÿ”n‹ğ…ötFhhVÿœnh„V‹øÿœn…ÿt…Àt‰=ˆ‰£Œ‰_^Ã3ÀPPjPÿ n£l‰…Àuçjè‡  ÌÌÌÌÌÌÌÌhp‰ÿ,n¡l‰…ÀtPÿnÃU‹ìëÿuè›  Y…Àtÿuè|0  Y…Àtæ]Ãƒ}ÿ„‘  éo  ;€òuòÃòé˜  U‹ìÿuè
   ÷ØYÀ÷ØH]ÃU‹ìƒ=œ‰ÿÿuuè&  ëhœ‰è¨%  Y÷ØYÀ÷Ğ#E]Ãjh`uèU  ƒeü ¸MZ  f9   u]¡<  ¸   PE  uL¹  f9ˆ  u>‹E¹   +ÁPQèF  YY…Àt'ƒx$ |!ÇEüşÿÿÿ°ë‹Eì‹ 3É8  À”Á‹ÁÃ‹eèÇEüşÿÿÿ2À‹Mğd‰    Y_^[ÉÃVè±  …Àt d¡   ¾”‰‹Pë;Ğt3À‹Êğ±…Àuğ2À^Ã°^ÃU‹ìè}  …Àt€} u	3À¹”‰‡]ÃU‹ìƒ} uÆ˜‰è  èL  „Àu2À]Ãèg  „Àu
j èS  Yëé°]ÃU‹ì€=˜‰ t€} uÿuèM  ÿuè*  YY°]ÃU‹ì€=™‰ t°]ÃV‹u…ötƒşubèå  …Àt&…öu"hœ‰èú#  Y…Àuh¨‰èë#  Y…Àt+2Àë0ƒÉÿ‰œ‰‰ ‰‰¤‰‰¨‰‰¬‰‰°‰Æ™‰°^]Ãjè  ÌU‹ìèx  …Àuƒ}uÿu‹MPÿuÿ  ÿUÿuÿuèC)  YY]Ãj è>ÿÿÿ„ÀY•ÀÃè9  …Àtè\  ëè%  PèF   Y…Àt2ÀÃè•&  °Ãè  …Àthœ‰èƒ#  YÃèk  …À„€  Ãj èQ  Yé  è  „Àu2ÀÃèL  „Àuè  ëí°ÃèD  è  °ÃU‹ì‹EV‹H<È·AQĞ·Akğ(ò;Öt‹M;Jr
‹BB;ÈrƒÂ(;Öuê3À^]Ã‹Âëùƒ%´‰ ÃU‹ìì$  SjèKÖ  …Àt‹MÍ)jè×ÿÿÿÇ$Ì  …Üüÿÿj Pè[  ƒÄ‰…Œıÿÿ‰ˆıÿÿ‰•„ıÿÿ‰€ıÿÿ‰µ|ıÿÿ‰½xıÿÿfŒ•¤ıÿÿfŒ˜ıÿÿfŒtıÿÿfŒ…pıÿÿfŒ¥lıÿÿfŒ­hıÿÿœ…œıÿÿ‹E‰…”ıÿÿE‰… ıÿÿÇ…Üüÿÿ  ‹@üjP‰…ıÿÿE¨j PèÑ  ‹EƒÄÇE¨  @ÇE¬   ‰E´ÿÔnj Xÿ÷ÛE¨‰Eø…ÜüÿÿÛ‰EüşÃÿoEøPÿ4o…Àu„ÛujèâşÿÿY[ÉÃÌÌÌÌÌÌÌÂ  ÌÌÌÌÌÌÌÌÌÌÌÌÌU‹ìV‹ñFÇ¤ƒ  ƒ` P‹EƒÀPè4  YY‹Æ^]Â ÌÌÌÌ‹A…Àu¸¬ÃÌÌÌU‹ìV‹ñFÇ¤Pèe  öEYt
jVèD  YY‹Æ^]Â ƒa ‹Áƒa ÇAÌÇÄÃÌÌÌÌÌÌÌÌÌÌÌAÇ¤Pè  YÃÌÌÌÌÌÌÌÌÌÌÌÌÌÌÌU‹ìVÿu‹ñèBÿÿÿÇÄ‹Æ^]Â ƒa ‹Áƒa ÇAèÇàÃÌÌÌÌÌÌÌÌÌÌÌÌÌU‹ìVÿu‹ñèÿÿÿÇà‹Æ^]Â U‹ìƒìMôèTÿÿÿh|uEôPèq  ÌU‹ìƒìMôè•ÿÿÿhĞuEôPèT  ÌU‹ìì$  jèÖÓ  …ÀtjYÍ)£ÀŠ‰¼Š‰¸Š‰´Š‰5°Š‰=¬ŠfŒØŠfŒÌŠfŒ¨ŠfŒ¤ŠfŒ% ŠfŒ-œŠœĞŠ‹E £ÄŠ‹E£ÈŠE£ÔŠ‹…ÜüÿÿÇŠ  ¡ÈŠ£Ì‰ÇÀ‰	 ÀÇÄ‰   ÇĞ‰   jXkÀ Ç€Ô‰   jXkÀ ‹€‰LøjXÁà ‹€‰Løh è   ÉÃU‹ìj ÿoÿuÿ4oh	 ÀÿlnPÿ o]ÃU‹ìƒ%ÜŒ ƒì$ƒ€j
èªÒ  …À„©  ƒeğ 3ÀSVW3É}ÜS¢‹ó[‰‰w‰O3É‰W‹EÜ‹}ä‰Eô÷ntel‹Eè5ineI‰Eø‹Eà5Genu‰Eü3À@S¢‹ó[]Ü‰‹Eü‰sÇEø‰K‰SuC‹EÜ%ğ?ÿ=À t#=` t=p t=P t=` t=p u‹=àŒƒÏ‰=àŒë‹=àŒ‹MäjX‰Mü9Eô|/3ÉS¢‹ó[]Ü‰‰s‰K‹Mü‰S‹]à÷Ã   tƒÏ‰=àŒë‹]ğ¡€ƒÈÇÜŒ   £€÷Á   „“   ƒÈÇÜŒ   £€÷Á   ty÷Á   tq3ÉĞ‰Eì‰Uğ‹Eì‹Mğj^#Æ;ÆuW¡€ƒÈÇÜŒ   £€öÃ t;ƒÈ ÇÜŒ   £€¸  Ğ#Ø;Øu‹Eìºà   ‹Mğ#Â;Âuƒ€@‰5ÜŒ_^[3ÀÉÃ3À@Ã3À9äŒ•ÀÃÌÌÌÌÌÌÌÌÌÌhğB gdÿ6  ‹D$‰l$l$+àSVW¡€1Eü3ÅP‰eèÿuø‹EüÇEüşÿÿÿ‰EøEğgd£  òÃ‹Mğgd‰  Y__^[‹å]QòÃU‹ìÿuè¶ôÿÿY]ÃÌÌÌÌÌÌÌÌÌÌU‹ìöEV‹ñÇt
jVèÎÿÿÿYY‹Æ^]Â jhvèaÿÿÿj èÜ÷ÿÿY„À„Ñ   è÷ÿÿˆEã³ˆ]çƒeü ƒ=‰ …Å   Ç‰   èÂøÿÿ„ÀtMè	  è¿  èÒ  h¸ih¤iè±!  YY…Àu)è£øÿÿ„Àt h ihœièN!  YYÇ‰   2Ûˆ]çÇEüşÿÿÿè=   „ÛuCè¨  ‹ğƒ> tVèPöÿÿY„Àtÿujÿu‹6‹Îÿ  ÿÖÿèŒ3À@ëŠ]çÿuãèçöÿÿYÃ3À‹Mğd‰    Y_^[ÉÃjèæøÿÿÌjh0vèZşÿÿ¡èŒ…À3ÀëiH£èŒ3ÿG‰}äƒeü èköÿÿˆEà‰}üƒ=‰ukèû÷ÿÿèÔ  è6  ƒ%‰ ƒeü è9   j ÿuè»öÿÿYY¶ğ÷Şö#÷‰uäÇEüşÿÿÿè"   ‹Æ‹Mğd‰    Y_^[ÉÃ‹}äÿuàè.öÿÿYÃ‹uäè¹÷ÿÿÃjè6øÿÿÌÌÌÌÌÌÌU‹ì‹Eƒè t3ƒèt ƒètƒèt3À@ë0è²÷ÿÿëèŒ÷ÿÿ¶ÀëÿuÿuèşÿÿYëƒ} •À¶ÀPèüşÿÿY]Â U‹ìV‹5…öu3À@ëÿu‹Îÿuÿuÿ  ÿÖ^]Â jhXvè&ıÿÿ‹}…ÿu9=èŒ3ÀéÙ   ƒeü ƒÿt
ƒÿt‹]ë1‹]SWÿuè“ÿÿÿ‹ğ‰uä…ö„£   SWÿuè)ÿÿÿ‹ğ‰uä…ö„Œ   SWÿuèRÛÿÿ‹ğ‰uäƒÿu'…öu#SPÿuè:Ûÿÿ…Û•À¶ÀPè6şÿÿYSVÿuè4ÿÿÿ…ÿtƒÿuHSWÿuèÎşÿÿ‹ğ‰uä…öt5SWÿuèÿÿÿ‹ğë$‹Mì‹Qÿ0hÀ3 ÿuÿuÿuè¶õÿÿƒÄÃ‹eè3ö‰uäÇEüşÿÿÿ‹Æ‹Mğd‰    Y_^[ÉÃÌÌÌÌÌÌÌU‹ìƒ}uè#  ÿuÿuÿuèÒşÿÿƒÄ]Â ¡ìŒS3ÛCV;Ãt`…ÀuXhÿ”n‹ğ…öu‹óë*h0Vÿœn…ÀtìhHV£ğŒÿœn…Àt×£ôŒ‹ÎºìŒ3Àğ±
…Àu;ót;ÃtŠÃë2À^[Ãèˆÿÿÿ„ÀtV‹5ğŒ‹ÎhøŒÿ  ÿÖ^ÃºøŒëó¡øŒ…Àuõ3ÉAğ±
…ÀuìÃèIÿÿÿ„ÀtV‹5ôŒ‹ÎhøŒÿ  ÿÖ^ÃÇøŒ    Ã‹ÿU‹ì‹<  SVWƒ¹t  vD‹±à  …öt:·  ‘  ‹¾  Ğ·™  3ö…Ût‹B;ør	‹JÁ;ørFƒÂ(;órè3À_^[]Â ‹E‰‹E‹J$‰‹B   ëâ‹ÿU‹ìƒì@EäjPÿuÿ<o…ÀujYÍ)öEøDtEVWEÀPÿ°n‹}Ä‹÷÷Ş#uWÿ‹Â#U#EJÇÂ3Ò÷÷3Ò‹È‹E÷÷Èt3Àğ	÷ƒéuô_^ÉÂ ‹ÿU‹ìQQVEøPEüPèÿÿÿ‹ğ…öu‹EÇ    ëCƒ= u!÷Eø   €Ç   ujYÍ)ÿuüVèAÿÿÿÿuÿuÿuüVÿ8o…ÀujYÍ)^ÉÂ ÷¸   t&è,şÿÿ¡üŒ@£üŒƒøuh jèeÿÿÿéJşÿÿÃ‹ÿU‹ìQ÷¸   t"èóıÿÿƒ-üŒuEüPÿ5 è0ÿÿÿèşÿÿÉÃ‹ÿU‹ìƒì4SVWè‡ÿÿÿ‹M¿   ÇEÌ$   ‰MĞ‹A‹YÇ‹Qß‹q×÷‰]ü‹yÇ   ‰EØ‰}ğ‹y‰}ø‹}‰}Ô3ÿ÷   ‰}Ü‰}à‰}ä‰}è‰}ìu$EÌ‰EüèRÿÿÿEüPjWhW mÀÿ o3Àé×  ‹E‹;+ÂÁø‰Eô‹†‹ÁÁè÷Ğƒà‰EÜ  u·Á‹3ö‰Eà…ÛtEÌ‹ËPVÿ  ÿÓ‹ğ…ö…\  ‹…ÿ…“   …ÛtEÌ‹ËPjÿ  ÿÓ‹ø…ÿua3ÛSSÿuØÿèn‹ø…ÿuNÿˆn‹= ‰Eì…ÿtEÌ‹ÏPjÿ  ÿ×‹ø…ÿu%EÌ‰EüèzşÿÿEüPjSh~ mÀÿ o‹Eèéş   ‹Mü‹Ç‡;ÇuWÿPn‹‰}ä…ÛtEÌ‹ËPjÿ  ÿÓ‹ğ…ö…•   ‹E3Û9Xt-9Xt(‹G<<8PE  u‹Mø9L8u;|84u‹uô‹Eğ‹4°…öu^ÿuàWÿœn‹ğ…öuNÿˆn‹5 ‰Eì…ötEÌ‹ÎPjÿ  ÿÖ‹ğ…öu%EÌ‰Eğè¬ıÿÿEğPjSh mÀÿ oèbıÿÿ‹uè‹E‰0‹…Ûtƒeì EÌPj‹Ë‰}ä‰uèÿ  ÿÓèdıÿÿ‹Æ_^[ÉÂ ‹€VW¿Næ@»¾  ÿÿ;Ït…Îu&è,   ‹È;Ïu¹Oæ@»ë…Îu
G  ÁàÈ‰€÷Ñ_‰€^ÃU‹ìƒìƒeô Eôƒeø Pÿ´n‹Eø3Eô‰Eüÿtn1Eüÿpn1EüEìPÿün‹EğMü3Eì3Eü3ÁÉÃhÿÌnÃhèÊ  YÃè   ‹Hƒ$‰Hè   ‹Hƒ‰HÃ¸Ã¸Ã¸ ÃSV¾øt»øt;ósW‹>…ÿt
‹Ïÿ  ÿ×ƒÆ;óré_^[ÃSV¾ u» u;ósW‹>…ÿt
‹Ïÿ  ÿ×ƒÆ;óré_^[ÃU‹ìƒì‹ESW‹}» “‰Eğ…ÿt-öt‹ƒéVQ‹‹p ‹Î‹xÿ  ÿÖ^…ÿt
öt» @™‹Eğ‰EøEôPjjhcsmà‰]ô‰}üÿ o_[ÉÂ U‹ìQ‹E‹MSV‹XW‹x‹×‰Uü‹ò…Éx-kÂƒÃÃ‹]ƒúÿt<ƒèJ9Xü};~ƒúÿu‹uüI‰Uü…ÉyŞB;÷w;Öw‹E‹M_‰p^‰‰P‰H[ÉÃè—&  ÌU‹ìQS‹EƒÀ‰Eüd‹    ‹d£    ‹E‹]‹mü‹cüÿà[ÉÂ U‹ìQQSVWd‹5    ‰uøÇEüî< j ÿuÿuüÿuÿo‹E‹@ƒàı‹M‰Ad‹=    ‹]ø‰;d‰    _^[ÉÂ U‹ìƒì¡€Mèƒeè 3Á‹M‰Eğ‹E‰Eô‹E@ÇEìı> ‰Mø‰Eüd¡    ‰EèEèd£    ÿuQÿuèª4  ‹È‹Eèd£    ‹ÁÉÃU‹ìƒì@S}#  u¸/> ‹M‰3À@éÁ   ƒeÀ ÇEÄ.? ¡€MÀ3Á‰EÈ‹E‰EÌ‹E‰EĞ‹E‰EÔ‹E ‰EØƒeÜ ƒeà ƒeä ‰eÜ‰màd¡    ‰EÀEÀd£    ÇEø   ‹E‰Eè‹E‰Eìè¹%  ‹@‰Eü¡  ‰Eô‹MüÿUô‹Eü‰EğEèP‹Eÿ0ÿUğYYƒeø ƒ}ä td‹    ‹‹]À‰d‰    ë	‹EÀd£    ‹Eø[ÉÃU‹ì‹MV‹u‰èP%  ‹H$‰NèE%  ‰p$‹Æ^]ÃU‹ìVè4%  ‹u;p$u‹vè$%  ‰p$^]Ãè%  ‹H$ƒÁë;ğtH‹…Àt	ëñ‹F‰ëÚèd$  ÌU‹ìƒìSVWü‰Eü3ÀPPPÿuüÿuÿuÿuÿuèD*  ƒÄ ‰Eø_^[‹Eø‹å]ÃU‹ìVü‹u‹N3Îè­éÿÿj Vÿvÿvj ÿuÿvÿuè	*  ƒÄ ^]ÃU‹ìQSü‹E‹H3Mèzéÿÿ‹E‹@ƒàft‹EÇ@$   3À@ëlëjj‹Eÿp‹Eÿp‹Eÿpj ÿu‹Eÿpÿuè¬)  ƒÄ ‹Eƒx$ uÿuÿuè(ıÿÿj j j j j EüPh#  èÁıÿÿƒÄ‹Eü‹]‹c‹k ÿà3À@[ÉÃÌÌÌÌÌjhxvè”ñÿÿ‹E…Àt~8csmàuvƒxupx “tx!“t	x"“uU‹H…ÉtN‹Q…Òt)ƒeü RÿpèJ   ÇEüşÿÿÿë1ÿuÿuìèä   YYÃ‹eèëäöt‹@‹…Ét‹Q‹p‹Îÿ  ÿÖ‹Mğd‰    Y_^[ÉÃU‹ì‹MÿU]Â U‹ìè6#  ‹@$…Àt‹M9t‹@…Àuõ3À@]Ã3À]ÃU‹ì‹M‹UV‹‹qÂ…öx‹I‹‹
ÎÁ^]ÃU‹ìV‹uW‹>?RCCàt?MOCàt
?csmàtëèÊ"  ƒx ~è¿"  ÿH_3À^]Ãè±"  ‰x‹vè¦"  ‰pè  ÌU‹ì€} t2VW‹}‹7>csmàu!ƒ~u~ “t~!“t~"“t_^3À]ÃèY"  ‰p‹wèN"  ‰pè·  ÌU‹ìW‹}€ tH‹…ÉtBQŠA„Àuù+ÊSVYSè  ‹ğY…ötÿ7SVè1  ‹E‹ÎƒÄ3ö‰Æ@Vè@  Y^[ë‹M‹‰ÆA _]ÃU‹ìV‹u€~ tÿ6è  Yƒ& ÆF ^]ÃU‹ìÿuÿĞn…ÀtV‹0Pèõ  ‹ÆY…öuñ^]ÃU‹ì‹E‹M;Áu3À]ÃƒÁƒÀŠ:u„ÒtìŠP:QuƒÀƒÁ„ÒuäëØÀƒÈ]Ãè¶0  „Àu2ÀÃè!  „Àuèİ0  ëí°ÃU‹ì€} u
è!  èÅ0  °]Ãè!  °Ãè/!  …À•ÀÃj è´!  Y°ÃÌÌÌÌU‹ìV‹uW‹}‹ƒøşt‹NÏ38èíåÿÿ‹F‹NÏ38_^]éÚåÿÿÌÌÌÌÌÌÌÌÌÌÌÌÌÌU‹ìƒìS‹]VWÆEÿ ÿ3ÇEô   èy¾  ‰‹]‹Cs3€VP‰uğ‰Eøè„ÿÿÿÿuè(/  ‹EƒÄ‹{ö@fuZ‰Eä‹E‰EèEä‰Cüƒÿşti‹MøGG‹‹H‰Eì…Ét‹ÖèU2  ±ˆMÿ…ÀxHëŠMÿ‹ûƒûşuÉ„Ét.ë ÇEô    ëƒÿşth€Vºşÿÿÿ‹Ëèh2  VÿuøèóşÿÿƒÄ‹Eô_^[‹å]Ã‹E8csmàu8ƒ=$ t/h$è(0  ƒÄ…Àt‹5$‹Îjÿuÿ  ÿÖ‹uğƒÄ‹E‹M‹Ğèé1  ‹E9xth€V‹×‹Èèî1  ‹EVÿuø‰Xèsşÿÿ‹MìƒÄ‹Ö‹Iè—1  ÌÌÌVè   ‹ğ…öt
‹Îÿ  ÿÖè¿  Ì¡ğÃÌÌÌÌÌÌÌÌÌÌÌÌÌÌ‹L$¶D$‹×‹|$…É„<  iÀƒù †ß   ù€   ‚‹   º%àŒs	óª‹D$‹úÃº%€ƒ²   fnÀfpÀ ÏƒÇƒçğ+Ïù€   vL¤$    ¤$    ffGfG fG0fG@fGPfG`fGp¿€   é€   ÷Á ÿÿÿuÅëº%€s>fnÀfpÀ ƒù róóGƒÇ ƒé ƒù sì÷Á   tb|àóóG‹D$‹úÃ÷Á   tˆGƒé÷Á   uò÷Á   t‰ƒÇƒé÷Áøÿÿÿt ¤$    ›    ‰‰GƒÇƒé÷Áøÿÿÿuí‹D$‹úÃU‹ìV3öƒ=ÜŒ}0‹M‹Ñ·ƒÁf…Àuõƒé·;Êtf;Euğëf;Eu‹Áëf3Àëb‹Uf‹Më·f;Áu‹òf…ÀtHƒÂB¨uç3Àf;Áu¸ ÿÿfnÈëƒÂf:cÈuòJë·ÁfnÀf:cAs4JtƒÂëî‹Æ^]ÃÌÌÇp“X…°ÃÌÌÌhHèç  Ç$TèÛ  Y°ÃÌÌÌÌÌÌ°ÃÌÌÌÌÌÌÌÌÌÌÌÌÌè  °ÃÌÌÌÌÌÌÌÌ‹ÿV‹5€Vè¤F  Vè|  VèG  VèJ  Vè  ƒÄ°^ÃÌÌj èpûÿÿYÃÌÌÌÌÌÌÌ‹ÿU‹ìQhL“MÿèÄ   °ÉÃÌÌÌÌÌÌÌÌÌ‹ÿVÿ5|“èõC  ÿ5€“3ö‰5|“èâC  ÿ54“‰5€“èÑC  ÿ58“‰54“èÀC  ƒÄ‰58“°^Ãh¨h(èŞC  YYÃ‹ÿU‹ì€} tƒ=x“ tè·I  °]Ãh¨h(èD  YY]Ã‹ÿU‹ìÿuèÁ3  Y°]Ãè(5  …À•ÀÃèÒ5  °Ã‹ÿU‹ìV‹uƒÉÿ‹ğÁuW¿8€9>t
ÿ6è'C  Y‰>_^]Â ‹ÿU‹ì¡€ƒàj Y+È‹EÓÈ3€]Ã‹ÿU‹ìVè1   ‹ğ…ötÿu‹Îÿ  ÿÖY…Àt3À@ë3À^]Ã‹ÿU‹ì‹E£,]Ãjh˜vèôèÿÿƒeä j èz2  Yƒeü ‹5€‹Îƒá35,ÓÎ‰uäÇEüşÿÿÿè   ‹Æ‹Mğd‰    Y_^[ÉÃ‹uäj èN2  YÃ¡4Ã‹ÿU‹ìj jÿuè  ƒÄ]Ãjj j èş   ƒÄÃ‹ÿU‹ì‹E£0]Ãj ÿ”n…Àt4¹MZ  f9u*‹H<È9PE  u¸  f9Auƒytvƒ¹è    t°Ã2ÀÃ‹ÿU‹ìQƒeü EüPh¨j ÿn…Àt#VhÀÿuüÿœn‹ğ…ötÿu‹Îÿ  ÿÖ^ƒ}ü t	ÿuüÿPnÉÃ‹ÿU‹ìè•K  ƒøt d¡0   ‹@hÁè¨uÿuÿlnPÿ oÿuèvÿÿÿYÿuÿ8nÌ‹ÿU‹ì3À}csmà”À]Ã‹ÿU‹ìƒìƒ} uèÿÿÿ„Àt	ÿuè9ÿÿÿYEÆEÿ ‰EèMşE‰EìEÿj‰EğX‰Eø‰EôEøPEèPEôPèë   ƒ} tÉÃÿuèHÿÿÿÌjh¸vèçÿÿ‹ñ€=8 …–   3À@¹4‡3Û‰]ü‹‹ …Àu,‹=€‹Ïƒá¡0;Çt3øÓÏSSS‹Ïÿ  ÿ×hHë
ƒøuhTè  YÇEüşÿÿÿ‹9uhÔihÄièú  YYhÜihØièé  YY‹F9uÆ8‹FÆ ‹Mğd‰    Y_^[ÉÃ‹Eì‹ ÿ0è¼şÿÿƒÄÃ‹eèèş  ÌjhØvè4æÿÿ‹Eÿ0è»/  Yƒeü ‹MèÿÿÿÇEüşÿÿÿè   ‹Mğd‰    Y_^[ÉÂ ‹Eÿ0è/  YÃ‹ÿU‹ì]éO   ‹ÿU‹ìV‹uşÿÿÿ?s9ƒÈÿ‹M3Ò÷u;Ès*¯MÁæ‹Æ÷Ğ;ÁvjPè7J  j ‹ğèh?  ƒÄ‹Æë3À^]Ã‹ÿU‹ìƒìS‹]…Ûu3Àé  VƒûtƒûtèoI  j^‰0èş?  ‹Æéú   WèË7  h  ¾@3ÿVWè`R  ¡<“ƒÄ‰5,“‰Eğ…Àt€8 u‹Æ‰uğMô‰}üQMü‰}ôQWWPè°   jÿuôÿuüèÿÿÿ‹ğƒÄ …öuèõH  j_‰8ë2EôPEüP‹Eü†PVÿuğèv   ƒÄƒûu‹EüH£0“‹Æ‹÷£4“‹ßëJEø‰}øPVè”I  ‹ØYY…Ût‹Eøë&‹Uø‹Ï‹Â9:t@A98uø‹Ç‰0“‰Eø‹ß‰4“Pè8>  Y‰}øVè.>  Y‹Ã_^[ÉÃ‹ÿU‹ìQ‹ES‹]V‹uWƒ# ‹}Ç    ‹E…Àt‰0ƒÀ‰E2ÉˆMÿ€?"u„É°"”ÁGˆMÿë5ÿ…ötŠˆFŠGˆEş