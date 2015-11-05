#ifndef LAZY_STRING_LAZY_STRING_H
#define LAZY_STRING_LAZY_STRING_H

#include <memory>
#include <string>
#include <cctype>
#include <exception>
#include <type_traits>
#define QUOTE(x) #x
#define LINE QUOTE(__LINE__)
#define PLACE __FILE__ ": " LINE

namespace std_utils{

template<class C, class T=std::char_traits<C>>
class basic_lazy_string;
struct ci_char_traits;
typedef basic_lazy_string<char> lazy_string;
typedef basic_lazy_string<wchar_t> lazy_wstring;
typedef basic_lazy_string<char, ci_char_traits> lazy_istring;

template<class C, class T>
void swap(basic_lazy_string<C, T>& lhs, basic_lazy_string<C, T>& rhs);

template<class C, class T>
class basic_lazy_string{
    template<class C1, class T1>
    friend void std_utils::swap(basic_lazy_string<C1, T1>& lhs, basic_lazy_string<C1, T1>& rhs);
    struct char_proxy;

public:
    typedef C value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const reference const_reference;
    typedef typename std::pointer_traits<pointer>::difference_type difference_type;
    typedef typename std::make_unsigned<difference_type>::type size_type;
    typedef T traits_type;

    class out_of_range_exception: public std::exception{
        const char* message;
    public:
        out_of_range_exception(const char* m): message(m){}
        const char* what() const noexcept override {return message;}
    };

    class size_limit_exceeded_exception: public std::exception{
        const char* message;
    public:
        size_limit_exceeded_exception(const char* m): message(m){}
        const char* what() const noexcept override {return message;}
    };

    basic_lazy_string() : mBuf(mNull, mEmptyDeleter()), mCapacity(), mSize() {}
    basic_lazy_string(const basic_lazy_string& other)            = default;
    basic_lazy_string& operator=(const basic_lazy_string& other) = default;
    basic_lazy_string(basic_lazy_string&& other): basic_lazy_string(){
        swap(other);
    }
    basic_lazy_string& operator=(basic_lazy_string&& other){
        swap(other);
        return *this;
    }

    /*implicit*/ basic_lazy_string(const_pointer src): basic_lazy_string(){
        size_type size = traits_type::length(src);
        basic_lazy_string tmp(size);
        traits_type::copy(tmp.mGetBuf(), src, size);
        tmp.mSetSize(size);
        swap(tmp);
    }

    basic_lazy_string(const value_type c, size_type size): basic_lazy_string(size){
        traits_type::assign(mGetBuf(), size, c);
        mSetSize(size);
    }

    const_pointer c_str() const{
        return mGetBuf();
    }

    bool empty() const{
        return mSize == 0;
    }

    size_type size() const{
        return mSize;
    }

    size_type capacity() const{
        return mCapacity;
    }

    void swap(basic_lazy_string& other){
        std_utils::swap(*this, other);
    }

    void clear(){
        if(mIsShared()){
            basic_lazy_string emptyString;
            swap(emptyString);
            return;
        }
        mSetSize(0);
    }

    void reserve(size_type newCap){
        if(newCap > capacity() || (mIsShared() && newCap != 0)){
            mReallocate(newCap);
        }
    }

    difference_type compare(const basic_lazy_string& other) const {
        return mCompare(c_str(), size(), other.c_str(), other.size());
    }

    difference_type compare(const_pointer str) const {
        return mCompare(c_str(), size(), str, traits_type::length(str));
    }

    basic_lazy_string& operator+=(const basic_lazy_string& other){
        mAppend(other.c_str(), other.size());
        return *this;
    }

    basic_lazy_string& operator+=(const value_type chr){
        mAppend(chr);
        return *this;
    }

    basic_lazy_string& operator+=(const pointer str){
        mAppend(str, traits_type::length(str));
        return *this;
    }

    value_type operator[](size_type idx) const {
        mCheckRange(idx, PLACE);
        return mGetBuf()[idx];
    }

    char_proxy operator[](size_type idx){
        mCheckRange(idx, PLACE);
        return char_proxy(*this, idx);
    }
private:
    typedef std::shared_ptr<value_type> buf_t;

    explicit basic_lazy_string(size_type capacity)
    {
        if(capacity > mMaxCapacity){
            throw size_limit_exceeded_exception(PLACE);
        }
        mBuf = buf_t(new value_type[capacity > mMinCapacity? capacity + 1 : mMinCapacity + 1],
                     std::default_delete<value_type[]>()),
        mCapacity = capacity > mMinCapacity ? capacity : mMinCapacity;
        mSetSize(0);
    }

    basic_lazy_string(const basic_lazy_string& other, size_type capacity)
            : basic_lazy_string(capacity > other.capacity() ? capacity : other.capacity())
    {
        traits_type::copy(mGetBuf(), other.mGetBuf(), other.size());
        mSetSize(other.size());
    }

    void mAppend(const value_type chr) {
        value_type len = size();
        if(len == capacity() || mIsShared()){
            mReallocate(capacity() + 1);
        }
        traits_type::assign(mGetBuf()[len], chr);
        mSetSize(len+1);
    }

    void mAppend(const_pointer str, size_type addSize) {
        if(addSize == 0) return;
        value_type newSize = size() + addSize;
        if(newSize > capacity() || mIsShared()){
            mReallocate(newSize);
        }
        traits_type::copy(mGetBuf() + size(), str, addSize);
        mSetSize(newSize);
    }

    static difference_type mCompare(const_pointer strLhs, size_type lenLhs,
                                    const_pointer strRhs, size_type lenRhs) {
        difference_type res = traits_type::compare(strLhs, strRhs, std::min(lenLhs, lenRhs));
        if(!res){
            res = difference_type(lenLhs - lenRhs);
        }
        return res;
    }
    
    pointer mGetBuf() const{ return mBuf.get();}

    bool mIsShared() const{ return !mBuf.unique();}

    void mReallocate(size_type newCapacity){
        size_type oldCapacity = capacity();
        if(newCapacity > oldCapacity && newCapacity < 2*oldCapacity){
            newCapacity = 2*oldCapacity;
        }
        basic_lazy_string copy(*this, newCapacity);
        swap(copy);
    }

    void mCheckRange(size_type pos, const char* message) const{
        if(pos >= mSize) throw out_of_range_exception(message);
    }

    void mSetSize(size_type newSize){
        mSize = newSize;
        mGetBuf()[newSize] = value_type();
    }

    buf_t mBuf;
    size_type mCapacity;
    size_type mSize;
    static value_type mNull[1];
    static const size_type mMaxCapacity = (size_type(-1)/sizeof(value_type) - 1)/4;
    static const size_type mMinCapacity = 31/sizeof(value_type);

    struct mEmptyDeleter{
        void operator()(value_type p[]){}
    };

    struct char_proxy{
        basic_lazy_string& parent;
        size_type pos;
        char_proxy(basic_lazy_string& parent, size_type pos)
                : parent(parent),
                  pos(pos)
        {}
        char_proxy& operator=(const value_type& val){
            if(parent.mIsShared()){
                parent.mReallocate(parent.capacity());
            }
            parent.mGetBuf()[pos] = val;
            return *this;
        }
        operator value_type(){
            return parent.mGetBuf()[pos];
        }
    };
};

template<class C, class T>
void swap(basic_lazy_string<C, T>& lhs, basic_lazy_string<C, T>& rhs){
    using std::swap;
    swap(lhs.mSize,     rhs.mSize);
    swap(lhs.mCapacity, rhs.mCapacity);
    swap(lhs.mBuf,      rhs.mBuf);
};

template<class C, class T>
typename basic_lazy_string<C, T>::value_type basic_lazy_string<C, T>::mNull[1];
    
template<class C, class T>
const typename basic_lazy_string<C, T>::size_type basic_lazy_string<C, T>::mMaxCapacity;

template<class C, class T>
const typename basic_lazy_string<C, T>::size_type basic_lazy_string<C, T>::mMinCapacity;

// operator <
template<class C, class T>
bool operator<(const basic_lazy_string<C, T>& lhs, const basic_lazy_string<C, T>& rhs){
    return lhs.compare(rhs) < 0;
}

template<class C, class T>
bool operator<(typename basic_lazy_string<C, T>::const_pointer lhs, const basic_lazy_string<C, T>& rhs){
    return rhs.compare(lhs) > 0;
}

template<class C, class T>
bool operator<(const basic_lazy_string<C, T>& lhs, typename basic_lazy_string<C, T>::const_pointer rhs){
    return lhs.compare(rhs) < 0;
}
// operator <=
template<class C, class T>
bool operator<=(const basic_lazy_string<C, T>& lhs, const basic_lazy_string<C, T>& rhs){
    return lhs.compare(rhs) <= 0;
}

template<class C, class T>
bool operator<=(typename basic_lazy_string<C, T>::const_pointer lhs, const basic_lazy_string<C, T>& rhs){
    return rhs.compare(lhs) >= 0;
}

template<class C, class T>
bool operator<=(const basic_lazy_string<C, T>& lhs, typename basic_lazy_string<C, T>::const_pointer rhs){
    return lhs.compare(rhs) <= 0;
}
// operator >
template<class C, class T>
bool operator>(const basic_lazy_string<C, T>& lhs, const basic_lazy_string<C, T>& rhs){
    return lhs.compare(rhs) > 0;
}

template<class C, class T>
bool operator>(typename basic_lazy_string<C, T>::const_pointer lhs, const basic_lazy_string<C, T>& rhs){
    return rhs.compare(lhs) < 0;
}

template<class C, class T>
bool operator>(const basic_lazy_string<C, T>& lhs, typename basic_lazy_string<C, T>::const_pointer rhs){
    return lhs.compare(rhs) > 0;
}
// operator >=
template<class C, class T>
bool operator>=(const basic_lazy_string<C, T>& lhs, const basic_lazy_string<C, T>& rhs){
    return lhs.compare(rhs) >= 0;
}

template<class C, class T>
bool operator>=(typename basic_lazy_string<C, T>::const_pointer lhs, const basic_lazy_string<C, T>& rhs){
    return rhs.compare(lhs) <= 0;
}

template<class C, class T>
bool operator>=(const basic_lazy_string<C, T>& lhs, typename basic_lazy_string<C, T>::const_pointer rhs){
    return lhs.compare(rhs) >= 0;
}
// operator ==
template<class C, class T>
bool operator==(const basic_lazy_string<C, T>& lhs, const basic_lazy_string<C, T>& rhs){
    return lhs.compare(rhs) == 0;
}

template<class C, class T>
bool operator==(typename basic_lazy_string<C, T>::const_pointer lhs, const basic_lazy_string<C, T>& rhs){
    return rhs.compare(lhs) == 0;
}

template<class C, class T>
bool operator==(const basic_lazy_string<C, T>& lhs, typename basic_lazy_string<C, T>::const_pointer rhs){
    return lhs.compare(rhs) == 0;
}
// operator !=
template<class C, class T>
bool operator!=(const basic_lazy_string<C, T>& lhs, const basic_lazy_string<C, T>& rhs){
    return lhs.compare(rhs) != 0;
}

template<class C, class T>
bool operator!=(typename basic_lazy_string<C, T>::const_pointer lhs, const basic_lazy_string<C, T>& rhs){
    return rhs.compare(lhs) != 0;
}

template<class C, class T>
bool operator!=(const basic_lazy_string<C, T>& lhs, typename basic_lazy_string<C, T>::const_pointer rhs){
    return lhs.compare(rhs) != 0;
}
// operator +
template<class C, class T>
basic_lazy_string<C, T>  operator+(const basic_lazy_string<C, T>& lhs, const basic_lazy_string<C, T>& rhs){
    basic_lazy_string<C, T> result;
    result.reserve(lhs.size() + rhs.size());
    result += lhs;
    result += rhs;
    return result;
}

template<class C, class T>
basic_lazy_string<C, T>  operator+(typename basic_lazy_string<C, T>::const_pointer lhs, const basic_lazy_string<C, T>& rhs){
    basic_lazy_string<C, T> result;
    result.reserve(basic_lazy_string<C, T>::traits_type::length(lhs) + rhs.size());
    result += lhs;
    result += rhs;
    return result;
}

template<class C, class T>
basic_lazy_string<C, T>  operator+(const basic_lazy_string<C, T>& lhs, typename basic_lazy_string<C, T>::const_pointer rhs){
    basic_lazy_string<C, T> result;
    result.reserve(lhs.size() + basic_lazy_string<C, T>::traits_type::length(rhs));
    result += lhs;
    result += rhs;
    return result;
}

template<class C, class T>
basic_lazy_string<C, T>  operator+(const typename basic_lazy_string<C, T>::value_type lhs, const basic_lazy_string<C, T>& rhs){
    basic_lazy_string<C, T> result;
    result.reserve(1 + rhs.size());
    result += lhs;
    result += rhs;
    return result;
}

template<class C, class T>
basic_lazy_string<C, T>  operator+(const basic_lazy_string<C, T>& lhs, const typename basic_lazy_string<C, T>::value_type rhs){
    basic_lazy_string<C, T> result;
    result.reserve(lhs.size() + 1);
    result += lhs;
    result += rhs;
    return result;
}

struct ci_char_traits : public std::char_traits<char> {
    static bool eq(char c1, char c2){
        return std::tolower(c1) == std::tolower(c2);
    }
    static bool lt(char c1, char c2){
        return std::tolower(c1) <  std::tolower(c2);
    }
    static int compare(const char* s1, const char* s2, size_t n){
        while(n-- != 0) {
            if(std::tolower(*s1) < std::tolower(*s2)) return -1;
            if(std::tolower(*s1) > std::tolower(*s2)) return 1;
            ++s1; ++s2;
        }
        return 0;
    }
    static const char* find(const char* s, int n, char a){
        auto const ua (std::tolower(a));
        while(n-- != 0)
        {
            if(std::tolower(*s) == ua)
                return s;
            s++;
        }
        return nullptr;
    }
};


}
#undef QUOTE
#undef LINE
#undef PLACE

#endif //LAZY_STRING_LAZY_STRING_H
