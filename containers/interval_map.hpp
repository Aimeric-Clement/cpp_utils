#ifndef UTL_INTERVAL_MAP_HPP
#define UTL_INTERVAL_MAP_HPP

#include <map>

namespace utl
{
    /**
     * @brief A std::map adapter that associates values with interval of keys.
     * 
     * @note Usage:
     * @code
     * interval_map<int, char> foo('.'); // Default background value is 'A'
     * foo.assign(1, 5, 'A'); // Assign 'A' to the interval [1, 5)
     * // print foreach from 0 to 8: ".AAAA...."
     * foo.assign(3, 7, 'B');
     * // print foreach from 0 to 8: ".AABBBB.."
     * const char at2 = foo[2]; // is 'A' 
     * @endcode
     */
    template<typename K, typename T>
    class interval_map
    {
    private:
        T m_default_background_value;
        std::map<K, T> m_map;
    
    public:   
        /**
        * @brief Constructs an interval_map with the default background value.
        * @param background_value The value to be used outside of the specified intervals.
        */
        interval_map(const T& background_value)
        : m_default_background_value(background_value)
        {}

        template <typename = std::enable_if_t<std::is_default_constructible_v<T>>>
        interval_map()
        : m_default_background_value()
        {}
    
        /**
         * @brief Assign the given value to the interval [key_begin, key_end).
         * 
         * @details It will overwrite any previously defined values that may overlap with the new interval.
         * The interval includes key_begin but is not inclusive of key_end.
         * If key_end is before key_begin; no assignment is performed
         */
        void assign(const K& key_begin, const K& key_end, const T& val) 
        {
            T background_value_begin = m_default_background_value;
            T background_value_end = m_default_background_value;
    
            // Check for empty interval
            if (!(key_begin < key_end))
                return;
    
            if (m_map.empty())
            {
                if (val == m_default_background_value)
                    return;
                else
                {
                    m_map.emplace(key_begin, val);
                    m_map.emplace(key_end, m_default_background_value);
                    return;
                }
            }
    
            /**
             * fill in the background :     ____________________________
             * before: [8,13)=C             ______________[   'C'  [____
             * insert: [3,10)=B             ____[    'B'    [___________
             * final:  [3,10)=B, [10,13[=C  ____[    'B'    [ 'C'  [____
             *                                              ^
             *                                        reinserted boundary
             */
            auto upper_it = m_map.upper_bound(key_end);
            if (upper_it != m_map.begin())
            {
                --upper_it; // iterator moved at the last overlapped interval
                background_value_end = upper_it->second;
                ++upper_it;
            }
    
            auto lower_it = m_map.lower_bound(key_begin);
            if (lower_it != m_map.begin())
            {
                --lower_it;
                if (!(key_begin < lower_it->first))
                    background_value_begin = lower_it->second;
                ++lower_it;
            }
    
            // Remove any key left within the interval
            m_map.erase(lower_it, upper_it);
    
            // Add keys for the assigned interval if required
            if(!(background_value_begin == val))
                m_map.emplace(key_begin, val);
    
            if(!(background_value_end == val))
                m_map.emplace(key_end, background_value_end);
        }
    
        // Value lookup from some point, that may be contained within a key interval
        [[nodiscard]] const T& operator[](K const& key) const
        {
            auto it = m_map.upper_bound(key);
            if (it == m_map.begin())
                return m_default_background_value;
            else
                return (std::prev(it))->second;
        }
    };
}
#endif // UTL_INTERVAL_MAP_HPP