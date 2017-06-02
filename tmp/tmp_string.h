#ifndef TMP_STRING_H
#define TMP_STRING_H
namespace tmp{

    struct sized_cstring
    {
        const int size;
        const char * data;
        template <size_t N>
        constexpr sized_cstring(const char (&c)[N]) : size(N), data(c)
        { }

        constexpr sized_cstring(const sized_cstring& s) : size(s.size), data(s.data)
        { }
        constexpr int get_size() const { return size; }
        constexpr const char * get_data() const { return data; }
    };
}

#endif // TMP_STRING_H
