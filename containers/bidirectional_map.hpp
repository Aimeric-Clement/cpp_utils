#ifndef UTL_BIDIRECTIONAL_MAP_HPP
#define UTL_BIDIRECTIONAL_MAP_HPP

#include <unordered_map>
namespace utl
{
    /**
     * @brief data-structure with efficient lookups in both ways: key to value and key from value
     * It is implemented as an adapter data-structure that solely ensures
     * that two cross-referencing std::unordered_map are bijective
     */
    template <typename K, typename T>
    class bidirectional_map
    {
    private:
        using direct_iterator       = typename std::unordered_map<K, const T*>::iterator;
        using invert_iterator       = typename std::unordered_map<T, const K*>::iterator;

        std::unordered_map<K, const T*> m_direct; /// contains the left part of the bimap
        std::unordered_map<T, const K*> m_invert; /// contains the right part of the bimap
        /**
         *  m_direct and m_invert are cross-referencing the keys
         *    direct: <K,  T*>
         *              ^ ╱
         *               ╳
         *              v ╲
         *    invert: <T,  K*>
         */

    public:
        using direct_const_iterator = typename std::unordered_map<K, const T*>::const_iterator;
        using invert_const_iterator = typename std::unordered_map<T, const K*>::const_iterator;
        
        bidirectional_map() : m_direct(), m_invert() {};
        
        template <typename I>
        bidirectional_map(const I iter_begin, const I iter_end)
        : bidirectional_map()
        {
            I it = iter_begin;
             while (it != iter_end)
            {
                insert(*it);
                it = std::next(it);
            }
        }
        
        bidirectional_map(std::initializer_list<std::pair<K, T>> init)
        : bidirectional_map(init.begin(), init.end()) {}
        
        [[nodiscard]] const std::unordered_map<K, const T*>& direct() const
        { return m_direct; }
        [[nodiscard]] const std::unordered_map<T, const K*>& invert() const
        { return m_invert; }
        
        std::pair<direct_const_iterator, bool>
        insert(const std::pair<K, T>& value)
        {
            direct_const_iterator found_direct = m_direct.find(value.first);
            if (found_direct != m_direct.cend())
                return {found_direct, false};
            
            invert_const_iterator found_invert = m_invert.find(value.second);
            if (found_invert != m_invert.cend())
                return {m_direct.find(*found_invert->second), false};
            const T* placeholder = nullptr;
            direct_iterator inserted_direct = m_direct.insert(std::make_pair(value.first, placeholder)).first;
            invert_const_iterator inserted_invert = m_invert.insert(std::make_pair(value.second, &inserted_direct->first)).first;
            inserted_direct->second= &inserted_invert->first;
            return {inserted_direct, true};
        }
        
        std::pair<direct_const_iterator, bool>
        emplace(const K& key, const T& value)
        {
            direct_const_iterator found_direct = m_direct.find(key);
            if (found_direct != m_direct.cend())
                return {found_direct, false};
            
            invert_const_iterator found_invert = m_invert.find(value);
            if (found_invert != m_invert.cend())
                return {m_direct.find(*found_invert->second), false};
            const T* placeholder = nullptr;
            direct_iterator inserted_direct = m_direct.emplace(key, placeholder).first;
            invert_const_iterator inserted_invert = m_invert.emplace(value, &inserted_direct->first).first;
            inserted_direct->second= &inserted_invert->first;
            return {inserted_direct, true};
        }
        
        size_t erase_direct(const K& at)
        {
            direct_const_iterator key_entry = m_direct.find(at);
            if(key_entry == m_direct.cend())
                return 0;
            invert_const_iterator value_entry = m_invert.find(*key_entry->second);
            m_direct.erase(key_entry);
            m_invert.erase(value_entry);
            return 1;
        }
        
        size_t erase_direct(direct_const_iterator it)
        {
            invert_const_iterator it_invert = m_invert.find(*it->second);
            direct_const_iterator next = m_direct.erase(it);
            m_invert.erase(it_invert);
            return next;
        }
        
        size_t erase_invert(const T& at)
        {
            invert_const_iterator value_entry = m_invert.find(at);
            if(value_entry == m_invert.cend())
                return 0;
            direct_const_iterator key_entry = m_direct.find(*value_entry->second);
            m_invert.erase(value_entry);
            m_direct.erase(key_entry);
            return 1;
        }
        
        size_t erase_invert(invert_const_iterator it)
        {
            invert_const_iterator it_direct = m_direct.find(*it->second);
            invert_const_iterator next = m_invert.erase(it);
            m_invert.erase(it_direct);
            return next;
        }
        
        [[nodiscard]] size_t size() const
        {
            return m_direct.size();
        }

        void clear()
        {
            m_direct.clear();
            m_invert.clear();
        }
    };
}
#endif // UTL_BIDIRECTIONAL_MAP_HPP