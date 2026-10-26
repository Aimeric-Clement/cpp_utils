#ifndef UTL_BLOOM_FILTER_HPP
#define UTL_BLOOM_FILTER_HPP

#include <array>
#include <bitset>
#include <functional>
#include <limits>

namespace utl
{
    template <typename T>
    class bloom_filter
    {
    private:
        using hash_canvas_t = std::bitset<2048>;
        hash_canvas_t m_canvas;

        std::array<unsigned char, 4> compute_splat(const T& val) const
        {
            const size_t hash = std::hash<T>{}(val);
            constexpr unsigned char shrink_ratio = static_cast<unsigned char>( std::numeric_limits<unsigned char>::max() / hash_canvas_t{}.size()+1);
            return {
                    (static_cast<unsigned char>( hash       ) / shrink_ratio)
                  , (static_cast<unsigned char>( hash >> 8  ) / shrink_ratio)
                  , (static_cast<unsigned char>( hash >> 16 ) / shrink_ratio)
                  , (static_cast<unsigned char>( hash >> 24 ) / shrink_ratio)
                };
        }

    public:
        bloom_filter() = default;
        ~bloom_filter() = default;

        bool contains(const T& val) const
        {
            const std::array<unsigned char, 4> splat = compute_splat(val);
            return m_canvas.test(splat[0])
                && m_canvas.test(splat[1])
                && m_canvas.test(splat[2])
                && m_canvas.test(splat[3]);
        }

        void insert(const T& val)
        {
            const std::array<unsigned char, 4> splat = compute_splat(val);
            m_canvas.set(splat[0]);
            m_canvas.set(splat[1]);
            m_canvas.set(splat[2]);
            m_canvas.set(splat[3]);
        }

        void clear()
        {
            m_canvas.reset();
        }
    };
}
#endif // UTL_BLOOM_FILTER_HPP