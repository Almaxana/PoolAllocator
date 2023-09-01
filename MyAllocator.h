#include <vector>

template <typename T>
class PoolAllocator;

struct Chunk {
    Chunk* next;
};


template<typename U>
class Pool {
public:
    size_t pool_size;
    size_t chunk_size;
    Chunk* body;
    Chunk* available;

public:

    friend class PoolAllocator<U>;

    Pool (size_t pool_size_, size_t chunk_size_) {

        pool_size = pool_size_;
        chunk_size = chunk_size_;
        body = (Chunk*)malloc(pool_size_);
        if (body == nullptr) {
            std::cout<<"malloc returned NULL\n";
            std::bad_alloc{};
        }

        Chunk* chunk = body;
        available = chunk;
        size_t chunks_number = pool_size / chunk_size;
        for (size_t i = 0; i < chunks_number - 1; ++i) {
            chunk->next = reinterpret_cast<Chunk*>(reinterpret_cast<int8_t*>(chunk) + chunk_size);
            chunk = chunk->next;
        }
        chunk->next = nullptr;
    }

    template<typename Tp>
    Pool (const Pool<Tp>& other_p) {

        pool_size = other_p.pool_size;
        chunk_size = other_p.chunk_size;
        body = (Chunk*) malloc(other_p.pool_size);
        if (body == nullptr) {
            std::cout<<"malloc returned NULL\n";
            std::bad_alloc{};
        }
        memcpy(body, other_p.body, pool_size);
        available = reinterpret_cast<Chunk*>(reinterpret_cast<int8_t*>(body) + (reinterpret_cast<int8_t*>(other_p.available) - reinterpret_cast<int8_t*>(other_p.body)));

        Chunk* chunk = available;
        Chunk* ot_p_chunk = other_p.available;
        size_t chunks_number = pool_size / chunk_size;
        for (size_t i = 0; i < chunks_number - 1; ++i) {
            chunk->next = reinterpret_cast<Chunk*>(reinterpret_cast<int8_t*>(chunk) + (reinterpret_cast<int8_t*>(ot_p_chunk->next) - reinterpret_cast<int8_t*>(ot_p_chunk)));
            ot_p_chunk = ot_p_chunk->next;
            chunk = chunk->next;
        }
        chunk->next = nullptr;
    }

    Pool (const Pool& other_p) {
        pool_size = other_p.pool_size;
        chunk_size = other_p.chunk_size;
        body = (Chunk*) malloc(other_p.pool_size);
        if (body == nullptr) {
            std::cout<<"malloc returned NULL copying  "<<&other_p<<"\n";
            std::bad_alloc{};
        }
        memcpy(body, other_p.body, pool_size);
        available = reinterpret_cast<Chunk*>(reinterpret_cast<int8_t*>(body) + (reinterpret_cast<int8_t*>(other_p.available) - reinterpret_cast<int8_t*>(other_p.body)));

        Chunk* chunk = available;
        Chunk* ot_p_chunk = other_p.available;
        size_t chunks_number = pool_size / chunk_size;
        for (size_t i = 0; i < chunks_number - 1; ++i) {
            chunk->next = reinterpret_cast<Chunk*>(reinterpret_cast<int8_t*>(chunk) + (reinterpret_cast<int8_t*>(ot_p_chunk->next) - reinterpret_cast<int8_t*>(ot_p_chunk)));
            ot_p_chunk = ot_p_chunk->next;
            chunk = chunk->next;
        }
        chunk->next = nullptr;

    }

    Pool() = default;

    ~Pool() {
        free(body);
    }
    

};


template <typename T>
class PoolAllocator {
public:

    using value_type      = T;

    std::vector<Pool<T>> pools;

    PoolAllocator(std::vector<Pool<int>>& vec) {
        pools.shrink_to_fit();
        for (int i = 0; i < vec.size(); ++i) {
            pools.push_back(vec[i]);
        }
    }

    PoolAllocator() = default;

    ~PoolAllocator() = default;

    template<typename Tp>
    explicit PoolAllocator(const PoolAllocator<Tp>& other_al) {
        for (auto& pool:other_al.pools) {
            pools.push_back(pool);
        }
    }

    explicit PoolAllocator(const PoolAllocator& other_al) {
        for (auto& pool:other_al.pools) {
            pools.push_back(pool);
        }
    }


    friend bool operator==(const PoolAllocator& lhs, const PoolAllocator& rhs) {

        return &lhs == &rhs;
    }

    friend bool operator!=(const PoolAllocator& lhs, const PoolAllocator& rhs) {

        return !(lhs == rhs);
    }

    T* allocate(size_t size) {
        for (auto& pool : pools) {
            if (pool.available != nullptr && pool.chunk_size >= size * sizeof (T)) {
                Chunk* chunk = pool.available;
                pool.available = pool.available->next;

                return reinterpret_cast<T*>(chunk);
            }
        }
        std::bad_alloc{};
    }


    void deallocate(void* chunk, size_t size) {
        for (auto& pool : pools) {
            int8_t* temp = reinterpret_cast<int8_t*>(chunk);
            if (reinterpret_cast<int8_t*>(chunk) >= reinterpret_cast<int8_t*>(pool.body) && reinterpret_cast<int8_t*>(chunk) < reinterpret_cast<int8_t*>(pool.body) + (pool.pool_size)) {
                reinterpret_cast<Chunk *>(chunk)->next = pool.available;
                pool.available = reinterpret_cast<Chunk *>(chunk);

                break;
            }
        }
    }

};
