#pragma once
#include <vector>

template< typename T > struct Initializer {
   T t;
   Initializer()
   :t() // ====> default construction, works for classes _and_ built-in
   {}
};

template<typename T>
class CDataStore {
    
    protected:
        Initializer<T> i;
        T * m_pData = nullptr;
        unsigned int m_nDataSize = 0;
        int m_nCurrentIndex = -1;
        bool m_bOverflow = false;
        unsigned int m_nMaxSize = 100;

public:
        CDataStore() {}
        

        CDataStore(unsigned int nSize) {
            setSize(nSize);
        }

        ~CDataStore() {
           if(m_pData) delete(m_pData);
        }

        /**
         * Setting the size
         * - will lose all existing data.
         */
        void setSize(unsigned int nSize) {
            if(nSize != m_nMaxSize) {
                m_nMaxSize = nSize;
                if(m_pData) delete(m_pData);
                m_pData = new T[nSize];
                clear();
            }
        }
        
        /**
         * Clear the array 
         */
        void clear() {
            m_nCurrentIndex = -1;
            m_bOverflow = false;
        }

        /**
         * Store data in the array (append)
         */
        void add(T oData) {
            // Move to next Position
            m_nCurrentIndex++;
            if(m_nCurrentIndex >= (int) m_nMaxSize) {
                m_nCurrentIndex = 0;
                m_bOverflow = true;
            }
            m_pData[m_nCurrentIndex] = oData;
        }

        /**
         * size of the internal buffer
         */
        unsigned long size() {
            return(m_nDataSize);
        }
        /**
         * Number of elements in the array..
         */
        unsigned long count() {
            unsigned long nCount = 0;
            if(m_nCurrentIndex > -1) {
               nCount = m_bOverflow ? m_nDataSize : m_nCurrentIndex + 1;
            }
            return(nCount);
        }

        /**
         * Get the element at position nPos (index)
         * If the requested index is greater than the size of the data store,
         * it will be wrapped around, so nPos = 120, when size is 100, the
         * element at postion 20 will be returned.
         * If the data buffer is too small (1) or does not exist, the result is NULL
         * @param nPos zero based index
         * ENSURE with count() that data is inside - otherwise an exception occours
         */
        T getAt(unsigned long nPos) {
            T oResult= i.t;
            if(count() > 0) {
                // Find pos, relativ to start index
                // and adjust into array window...
                unsigned int nIndex = nPos;
                if(m_bOverflow) nPos = m_nCurrentIndex + nPos + 1;
                while(nIndex >= m_nDataSize) nIndex -= m_nDataSize;
                return(m_pData[nIndex]);
            }
            return oResult;
        }

        
};