//
// Created by smallville7123 on 5/08/20.
//

#ifndef AnyOpt_H
#define AnyOpt_H
#include <iostream>
#include <memory>
#include <string.h>
#include <assert.h>

// if ANYOPT_RUNTIME_ASSERTION is defined
// then AnyOpt uses runtime assertions instead of compile-time assertions

#define RUNTIME_WARNING_(msg) fprintf(\
        stderr,\
        "--- WARNING TRIGGERED ---\n"\
        "--- LOCATION: %s:%d ---\n"\
        "--- FUNCTION: %s ---\n"\
        "--- MESSAGE BEGIN ---\n"\
        "%s\n"\
        "--- MESSAGE END ---\n"\
        , __FILE__, __LINE__, __PRETTY_FUNCTION__, msg\
    ); \
    fflush(stderr)

#define RUNTIME_WARNING(expr, msg) if (!(expr)) { \
    RUNTIME_WARNING_(msg); \
}

#define RUNTIME_WARNING_EXECUTE_ADDITIONAL_CODE(expr, msg, code_block) if (!(expr)) { \
    RUNTIME_WARNING_(msg); \
    code_block \
}

#define RUNTIME_ASSERTION_(msg) fprintf(\
        stderr,\
        "--- ASSERTION TRIGGERED ---\n"\
        "--- LOCATION: %s:%d ---\n"\
        "--- FUNCTION: %s ---\n"\
        "--- MESSAGE BEGIN ---\n"\
        "%s\n"\
        "--- MESSAGE END ---\n"\
        , __FILE__, __LINE__, __PRETTY_FUNCTION__, msg\
    ); \
    fflush(stderr); \
    abort()

#define RUNTIME_ASSERTION(expr, msg) if (!(expr)) { \
    RUNTIME_ASSERTION_(msg); \
}

#ifdef ANYOPT_RUNTIME_ASSERTION
#define ASSERT_CODE(expr, msg) RUNTIME_ASSERTION(expr, msg)
#define ensure_flag_enabled(FLAGS, flag, message)         ASSERT_CODE(flag_is_set(FLAGS, flag), \
message "\n" \
"you can set it by appending\n" \
"|" #flag "\n" \
"to the flag list:\n" \
"AnyOptCustomFlags<Your_Flags|" #flag "> Your_Variable_Name;" \
)
#else
#define ASSERT_CODE(expr, msg) static_assert(expr, msg)
#define ensure_flag_enabled(FLAGS, flag, message) ASSERT_CODE(flag_is_set(FLAGS, flag), \
"\n" message "\n" \
"you can set it by appending\n" \
"|" #flag "\n" \
"to the flag list:\n" \
"AnyOptCustomFlags<Your_Flags|" #flag "> Your_Variable_Name;" \
)
#endif

#define AnyOpt_GTEST_ASSERT_DEATH(code_block, flag_that_must_be_enabled) \
    ASSERT_DEATH(code_block, #flag_that_must_be_enabled)


#define AnyOpt_Catch_Flag_POSIX_REGEX(FLAG_NAME) #FLAG_NAME

// const AnyOpt a(new int, true);
// alternative to
// const AnyOpt a; a.store(new int, true);

#define AnyOpt_FLAG_COPY_ONLY (1 << 0)
#define AnyOpt_FLAG_MOVE_ONLY (1 << 1)
#define AnyOpt_FLAG_COPY_ONLY_AND_MOVE_ONLY (AnyOpt_FLAG_COPY_ONLY | AnyOpt_FLAG_MOVE_ONLY)
#define AnyOpt_FLAG_ENABLE_CONVERSION_OF_ALLOCATION_COPY_TO_ALLOCATION_MOVE (1 << 2)
#define AnyOpt_FLAG_ENABLE_NON_POINTERS (1 << 3)
#define AnyOpt_FLAG_ENABLE_OPTIONAL_VALUE (1 << 4)
#define AnyOpt_FLAG_ENABLE_POINTERS (1 << 5)
#define AnyOpt_FLAG_DEBUG (1 << 6)

#define AnyOpt_FLAGS_DEFAULT (AnyOpt_FLAG_COPY_ONLY_AND_MOVE_ONLY | \
                                 AnyOpt_FLAG_ENABLE_CONVERSION_OF_ALLOCATION_COPY_TO_ALLOCATION_MOVE | \
                                 AnyOpt_FLAG_ENABLE_OPTIONAL_VALUE | \
                                 AnyOpt_FLAG_ENABLE_POINTERS | \
                                 AnyOpt_FLAG_ENABLE_NON_POINTERS)

// most of the time, this is the function you want
#define flag_is_set(flags, flag) ((flags & flag) != 0)
#define flag_is_not_set(flags, flag) ((flags & flag) == 0)

#define enableFunctionIf(T, E) template<class T, typename = typename std::enable_if<E, T>::type>
#define enableFunctionIfFlagIsSet(T, FLAGS, FLAG) enableFunctionIf(T, (FLAGS & FLAG) == 0)
#define enableFunctionIfFlagIsNotSet(T, FLAGS, FLAG) enableFunctionIf(T, (FLAGS & FLAG) != 0)

class AnyNullOpt_t {
public:
    AnyNullOpt_t() {}
    AnyNullOpt_t(const AnyNullOpt_t &x) {}
    AnyNullOpt_t(AnyNullOpt_t &&x) {}

    AnyNullOpt_t &operator=(const AnyNullOpt_t &x)  {
        puts("AnyNullOpt_t copy assignment");
        fflush(stdout);
        return *const_cast<AnyNullOpt_t*>(this);
    }

    AnyNullOpt_t &operator=(AnyNullOpt_t &&x)  {
        puts("AnyNullOpt_t move assignment");
        fflush(stdout);
        return *const_cast<AnyNullOpt_t*>(this);
    }
};

#define AnyNullOpt AnyNullOpt_t()

template <int FLAGS> class AnyOptCustomFlags {
public:
    
    class dummy {
    public:
        virtual dummy *clone() const = 0;
        virtual ~dummy() {};
        bool isDebug = false;
    
        void enableDebug() {
            isDebug = true;
        }
    
        void disableDebug() {
            isDebug = false;
        }
    };

    template<typename T>
    class storage : public dummy {
    public:
        T* data = nullptr;

        bool pointer_is_allocated = false;
        bool is_pointer = false;
        bool is_clone_move = false;

        template<class P = void, typename = typename std::enable_if<std::is_same<typename std::remove_reference<T>::type, void>::value, P>::type>
        storage * clone_impl(P * unused1 = nullptr, P * unused2 = nullptr) const {
            bool A = std::is_same<typename std::remove_reference<T>::type, void>::value;
            RUNTIME_ASSERTION(
                    A,
                    "this function cannot be invoked for non void types"
            )
            if (dummy::isDebug) {
                puts("AnyOptCustomFlags::storage clone void pointer");
                printf(
                        "AnyOptCustomFlags::storage clone void pointer - is_pointer: %s\n",
                        is_pointer ? "true" : "false"
                );
                printf(
                        "AnyOptCustomFlags::storage clone void pointer - pointer_is_allocated: %s\n",
                        pointer_is_allocated ? "true" : "false"
                );
                fflush(stdout);
            }
            // if data is allocated, a double free will occur
            if (pointer_is_allocated || is_clone_move) {
                ensure_flag_enabled(
                        FLAGS,
                        AnyOpt_FLAG_ENABLE_CONVERSION_OF_ALLOCATION_COPY_TO_ALLOCATION_MOVE,
                        "allocated void pointers CANNOT be copied and must be moved instead"
                        " however the ability to convert a data copy into a data move has not been"
                        " granted"
                );
                ensure_flag_enabled(
                        FLAGS,
                        AnyOpt_FLAG_MOVE_ONLY,
                        "allocated void pointers CANNOT be copied and must be moved instead"
                        " however the ability to move data has not been granted"
                );
                if (dummy::isDebug) {
                    puts(
                            "storage is being moved "
                            "because it has been marked as allocated and the "
                            "AnyOpt_FLAG_MOVE_ONLY "
                            "and "
                            "AnyOpt_FLAG_ENABLE_CONVERSION_OF_ALLOCATION_COPY_TO_ALLOCATION_MOVE "
                            "flags are set, and void pointers cannot be copied by dereference"
                    );
                    fflush(stdout);
                }
                // allocated
                storage *A_ = new storage(data, pointer_is_allocated, dummy::isDebug);
                const_cast<storage<T> *>(this)->data = nullptr;
                const_cast<storage<T> *>(this)->pointer_is_allocated = false;
                const_cast<storage<T> *>(this)->is_pointer = false;
                return A_;
            } else {
                // not allocated
                return new storage(data, pointer_is_allocated, dummy::isDebug);
            }
        }

        template<class P = void, typename = typename std::enable_if<!std::is_same<typename std::remove_reference<T>::type, void>::value && std::is_copy_constructible<typename std::remove_reference<T>::type>::value, P>::type>
        storage * clone_impl(P * unused1 = nullptr) const {
            if (dummy::isDebug) {
                puts("AnyOptCustomFlags::storage clone");
                fflush(stdout);
            }
            // if data is allocated, a double free will occur
            if (pointer_is_allocated || is_clone_move) {
                ensure_flag_enabled(
                        FLAGS,
                        AnyOpt_FLAG_ENABLE_CONVERSION_OF_ALLOCATION_COPY_TO_ALLOCATION_MOVE,
                        "allocated void pointers CANNOT be copied and must be moved instead"
                        " however the ability to convert a data copy into a data move has not been"
                        " granted"
                );
                ensure_flag_enabled(
                        FLAGS,
                        AnyOpt_FLAG_MOVE_ONLY,
                        "allocated void pointers CANNOT be copied and must be moved instead"
                        " however the ability to move data has not been granted"
                );
                if (dummy::isDebug) {
                    puts(
                            "storage is being moved "
                            "because it has been marked as allocated and the "
                            "AnyOpt_FLAG_MOVE_ONLY "
                            "and "
                            "AnyOpt_FLAG_ENABLE_CONVERSION_OF_ALLOCATION_COPY_TO_ALLOCATION_MOVE "
                            "flags are set, and void pointers cannot be copied by dereference"
                    );
                    fflush(stdout);
                }
                // allocated
                storage<T> * obj = new storage<T>(data, true, dummy::isDebug);
                const_cast<storage<T>*>(this)->data = nullptr;
                const_cast<storage<T>*>(this)->is_pointer = false;
                const_cast<storage<T>*>(this)->pointer_is_allocated = false;
                return obj;
            } else {
                // not allocated
                const T & data_ = *data;
                return new storage(data_, dummy::isDebug);
            }
        }

        template<class P = void, typename = typename std::enable_if<!std::is_same<typename std::remove_reference<T>::type, void>::value && !std::is_copy_constructible<typename std::remove_reference<T>::type>::value, P>::type>
        storage * clone_impl(P * unused1 = nullptr, P * unused2 = nullptr, P * unused3 = nullptr) const {
            if (dummy::isDebug) {
                puts("AnyOptCustomFlags::storage clone");
                fflush(stdout);
            }
            ensure_flag_enabled(
                    FLAGS,
                    AnyOpt_FLAG_ENABLE_CONVERSION_OF_ALLOCATION_COPY_TO_ALLOCATION_MOVE,
                    "allocated void pointers CANNOT be copied and must be moved instead"
                    " however the ability to convert a data copy into a data move has not been"
                    " granted"
            );
            ensure_flag_enabled(
                    FLAGS,
                    AnyOpt_FLAG_MOVE_ONLY,
                    "allocated void pointers CANNOT be copied and must be moved instead"
                    " however the ability to move data has not been granted"
            );
            if (dummy::isDebug) {
                puts(
                        "storage is being moved "
                        "because it has been marked as allocated and the "
                        "AnyOpt_FLAG_MOVE_ONLY "
                        "and "
                        "AnyOpt_FLAG_ENABLE_CONVERSION_OF_ALLOCATION_COPY_TO_ALLOCATION_MOVE "
                        "flags are set, and void pointers cannot be copied by dereference"
                );
                fflush(stdout);
            }
            // allocated or non copyable
            storage<T> * obj = new storage<T>(data, pointer_is_allocated, dummy::isDebug);
            const_cast<storage<T>*>(this)->data = nullptr;
            const_cast<storage<T>*>(this)->is_pointer = false;
            const_cast<storage<T>*>(this)->pointer_is_allocated = false;
            return obj;
        }

        virtual storage * clone() const override {
            return clone_impl();
        }

        template<class P, typename = typename std::enable_if<!std::is_same<typename std::remove_reference<T>::type, void>::value, P>::type>
        storage(const P &x, bool debug) {
            dummy::isDebug = debug;
            ensure_flag_enabled(
                    FLAGS,
                    AnyOpt_FLAG_COPY_ONLY,
                    "this function is not allowed unless the copy flag is set"
            );
            if (dummy::isDebug) {
                puts("AnyOptCustomFlags::storage copy constructor");
                fflush(stdout);
            }
            if (data == nullptr) {
                if (dummy::isDebug) {
                    puts("AnyOptCustomFlags::storage allocating and assigning data");
                    fflush(stdout);
                }
                const_cast<storage<T>*>(this)->data = new T(x);
                const_cast<storage<T>*>(this)->pointer_is_allocated = true;
                if (dummy::isDebug) {
                    puts("AnyOptCustomFlags::storage allocated and assigned data");
                    fflush(stdout);
                }
            } else {
                if (dummy::isDebug) {
                    puts("AnyOptCustomFlags::storage assigning data");
                    fflush(stdout);
                }
                *const_cast<storage<T>*>(this)->data = x;
                if (dummy::isDebug) {
                    puts("AnyOptCustomFlags::storage assigned data");
                    fflush(stdout);
                }
            }
        }

        template<class P, typename = typename std::enable_if<!std::is_same<typename std::remove_reference<T>::type, void>::value, P>::type>
        storage(P &&x, bool debug) : is_clone_move(true) {
            dummy::isDebug = debug;
            ensure_flag_enabled(
                    FLAGS,
                    AnyOpt_FLAG_MOVE_ONLY,
                    "this function is not allowed unless the move flag is set"
            );
            if (dummy::isDebug) {
                puts("AnyOptCustomFlags::storage move constructor");
                fflush(stdout);
            }
            if (data == nullptr) {
                const_cast<storage<T>*>(this)->data = new T(std::move(x));
                const_cast<storage<T>*>(this)->pointer_is_allocated = true;
            } else {
                std::swap(*data, x);
            }
        }

        storage(T * x, bool allocation, bool debug) {
            dummy::isDebug = debug;
            ensure_flag_enabled(
                    FLAGS,
                    AnyOpt_FLAG_ENABLE_POINTERS,
                    "this function is not allowed unless pointers are enabled"
            );
            if (dummy::isDebug) {
                puts("AnyOptCustomFlags::storage pointer constructor");
                fflush(stdout);
            }
            data = x;
            is_pointer = true;
            pointer_is_allocated = allocation;
        }

        template<class P, typename = typename std::enable_if<!std::is_same<typename std::remove_reference<T>::type, void>::value, P>::type>
        storage &operator=(const P &x)  {
            ensure_flag_enabled(
                    FLAGS,
                    AnyOpt_FLAG_COPY_ONLY,
                    "this assignment operator is not allowed unless the copy flag is set"
            );
            if (dummy::isDebug) {
                puts("AnyOptCustomFlags::storage copy assignment");
                fflush(stdout);
            }
            if (data == nullptr) {
                const_cast<storage<T>*>(this)->data = new T(x);
                const_cast<storage<T>*>(this)->pointer_is_allocated = true;
            } else
                *const_cast<storage<T>*>(this)->data = x;
            return *const_cast<storage<T>*>(this);
        }

        template<class P, typename = typename std::enable_if<!std::is_same<typename std::remove_reference<T>::type, void>::value, P>::type>
        storage &operator=(P &&x)  {
            ensure_flag_enabled(
                    FLAGS,
                    AnyOpt_FLAG_MOVE_ONLY,
                    "this assignment operator is not allowed unless the move flag is set"
            );
            if (dummy::isDebug) {
                puts("AnyOptCustomFlags::storage move assignment");
                fflush(stdout);
            }
            if (data == nullptr) {
                const_cast<storage<T>*>(this)->data = new T(std::move(x));
                const_cast<storage<T>*>(this)->pointer_is_allocated = true;
            }
            else
                std::swap(*data, x);
            return *const_cast<storage<T>*>(this);
        }

        storage &operator=(T * &x)  {
            ensure_flag_enabled(
                    FLAGS,
                    AnyOpt_FLAG_ENABLE_POINTERS,
                    "this assignment operator is not allowed unless pointers are enabled"
            );
            if (dummy::isDebug) {
                puts("AnyOptCustomFlags::storage pointer assignment");
                fflush(stdout);
            }
            return *const_cast<storage<T>*>(this);
        }

        // this MUST be able to be called regardless of stored type
        void deallocate() const {
            // an allocation could be cast to void* in order to avoid calling destructors
            // upon deletion
            if (pointer_is_allocated) {
                if (is_pointer) {
                    if (dummy::isDebug) {
                        puts("AnyOptCustomFlags::storage data an allocated pointer");
                        fflush(stdout);
                    }
                    if (data != nullptr) {
                        delete data;
                        const_cast<storage<T>*>(this)->data = nullptr;
                    } else {
                        if (dummy::isDebug) {
                            puts("AnyOptCustomFlags::storage data is an allocated pointer however it is assigned to nullptr, this is likely a bug");
                            fflush(stdout);
                        }
                    }
                } else {
                    if (dummy::isDebug) {
                        puts("AnyOptCustomFlags::storage data is a synthetic allocated pointer");
                        fflush(stdout);
                    }
                    if (data != nullptr) {
                        delete data;
                        const_cast<storage<T>*>(this)->data = nullptr;
                    } else {
                        if (dummy::isDebug) {
                            puts("AnyOptCustomFlags::storage data is a synthetic allocated pointer however it is assigned to nullptr, this is likely a bug");
                            fflush(stdout);
                        }
                    }
                }
            } else {
                if (is_pointer) {
                    if (dummy::isDebug) {
                        puts("AnyOptCustomFlags::storage data a pointer however it is not allocated");
                        fflush(stdout);
                    }
                } else {
                    if (dummy::isDebug) {
                        puts("AnyOptCustomFlags::storage data a synthetic pointer however it is not allocated, this is a bug");
                        fflush(stdout);
                    }
                }
            }
        }

        ~storage() override {
            if (dummy::isDebug) {
                puts("AnyOptCustomFlags::storage destructor ");
                fflush(stdout);
            }
            deallocate();
        }
    };

    dummy *data = nullptr;
    bool isAnyNullOpt = false;
    bool data_is_allocated = false;
    bool isDebug = false;
    
    void enableDebug() {
        isDebug = true;
        if (data != nullptr) data->enableDebug();
    }
    
    void disableDebug() {
        isDebug = false;
        if (data != nullptr) data->disableDebug();
    }
    
    bool has_value() const {
        return !isAnyNullOpt || data != nullptr;
    }

    AnyOptCustomFlags(const AnyNullOpt_t & opt) {
        if (flag_is_set(FLAGS, AnyOpt_FLAG_DEBUG)) isDebug = true;
        if (isDebug) {
            puts("AnyOptCustomFlags AnyNullOpt_t copy assignment");
            fflush(stdout);
        }
        deallocate();
    }

    AnyOptCustomFlags(AnyNullOpt_t && opt) {
        if (flag_is_set(FLAGS, AnyOpt_FLAG_DEBUG)) isDebug = true;
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_ENABLE_POINTERS,
                "this function is not allowed unless pointers are enabled"
        );
        if (isDebug) {
            puts("AnyOptCustomFlags nullptr_t move assignment");
            fflush(stdout);
        }
        deallocate();
    }

    AnyOptCustomFlags(const nullptr_t & opt) {
        if (flag_is_set(FLAGS, AnyOpt_FLAG_DEBUG)) isDebug = true;
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_ENABLE_POINTERS,
                "this function is not allowed unless pointers are enabled"
        );
        if (isDebug) {
            puts("AnyOptCustomFlags nullptr_t copy assignment");
            fflush(stdout);
        }
        deallocate();
    }

    AnyOptCustomFlags(nullptr_t && opt) {
        if (flag_is_set(FLAGS, AnyOpt_FLAG_DEBUG)) isDebug = true;
        if (isDebug) {
            puts("AnyOptCustomFlags nullptr_t move assignment");
            fflush(stdout);
        }
        deallocate();
    }

    void move(const AnyOptCustomFlags * obj) const {
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_MOVE_ONLY,
                "this function is not allowed unless the move flag is set"
        );

        AnyOptCustomFlags* this_ = const_cast<AnyOptCustomFlags*>(this);
        AnyOptCustomFlags* obj_ = const_cast<AnyOptCustomFlags*>(obj);

        deallocate();

        if (obj_->data != nullptr) {
            this_->data = obj_->data;
            this_->data->isDebug = isDebug;
        }
        this_->isAnyNullOpt = obj_->isAnyNullOpt;
        this_->data_is_allocated = obj_->data_is_allocated;

        obj_->data = nullptr;
        obj_->isAnyNullOpt = true;
        obj_->data_is_allocated = false;
    }

    void copy(const AnyOptCustomFlags * obj) const {
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_COPY_ONLY,
                "this function is not allowed unless the copy flag is set"
        );

        AnyOptCustomFlags* this_ = const_cast<AnyOptCustomFlags*>(this);

        deallocate();

        if (obj->data != nullptr) {
            this_->data = obj->data->clone();
            this_->data->isDebug = isDebug;
        }
        this_->isAnyNullOpt = false;
        this_->data_is_allocated = false;
    }

    template<class T, typename = typename std::enable_if<!std::is_same<typename std::remove_reference<T>::type, void>::value, T>::type>
    void store_copy(const T & what, const char * type) const {
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_COPY_ONLY,
                "this function is not allowed unless the copy flag is set"
        );
        bool A = std::is_same<typename std::remove_reference<T>::type, AnyOptCustomFlags>::value;
        bool B = std::is_same<typename std::remove_reference<T>::type, const AnyOptCustomFlags>::value;
        if (isDebug) {
            printf("AnyOptCustomFlags copy %s\n", type);
            fflush(stdout);
        }
        if (A || B) {
            const AnyOptCustomFlags * constobj = reinterpret_cast<const AnyOptCustomFlags*>(&what);
            AnyOptCustomFlags * obj = const_cast<AnyOptCustomFlags*>(constobj);
            if (
                obj->data &&
                obj->data_is_allocated &&
                flag_is_set(
                    FLAGS,
                    AnyOpt_FLAG_ENABLE_CONVERSION_OF_ALLOCATION_COPY_TO_ALLOCATION_MOVE
                )
            ) {
                ensure_flag_enabled(
                        FLAGS,
                        AnyOpt_FLAG_MOVE_ONLY,
                        "AnyOptCustomFlags is eligible for moving,"
                        " however it has not been granted the ability to move data"
                );
                if (isDebug) {
                    puts(
                            "AnyOptCustomFlags is being moved "
                            "because it has been marked as allocated and the "
                            "AnyOpt_FLAG_MOVE_ONLY "
                            "and "
                            "AnyOpt_FLAG_ENABLE_CONVERSION_OF_ALLOCATION_COPY_TO_ALLOCATION_MOVE "
                            "flags are set"
                    );
                    fflush(stdout);
                }
                store_move(*obj, type);
            } else {
                deallocate();
                if (isDebug) {
                    puts("AnyOptCustomFlags copying data");
                    fflush(stdout);
                }
                copy(reinterpret_cast<const AnyOptCustomFlags *>(&what));
                if (isDebug) {
                    puts("AnyOptCustomFlags copied data");
                    fflush(stdout);
                }
            }
        } else {
            deallocate();
            if (isDebug) {
                puts("AnyOptCustomFlags allocating and assigning data");
                fflush(stdout);
            }
            const_cast<AnyOptCustomFlags*>(this)->data = new storage<typename std::remove_reference<T>::type>(what, isDebug);
            if (isDebug) {
                puts("AnyOptCustomFlags allocated and assigned data");
                fflush(stdout);
            }
            const_cast<AnyOptCustomFlags*>(this)->isAnyNullOpt = false;
            const_cast<AnyOptCustomFlags*>(this)->data_is_allocated = true;
        }
    }

    template<class T, typename = typename std::enable_if<!std::is_same<typename std::remove_reference<T>::type, void>::value, T>::type>
    void store_move(T && what, const char * type) const {
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_MOVE_ONLY,
                "this function is not allowed unless the move flag is set"
        );
        bool A = std::is_same<typename std::remove_reference<T>::type, AnyOptCustomFlags>::value;
        bool B = std::is_same<typename std::remove_reference<T>::type, const AnyOptCustomFlags>::value;
        if (isDebug) {
            printf("AnyOptCustomFlags move %s\n", type);
            fflush(stdout);
        }
        if (A || B) {
            if (isDebug) {
                puts("AnyOptCustomFlags moving data");
                fflush(stdout);
            }
            move(const_cast<AnyOptCustomFlags*>(reinterpret_cast<const AnyOptCustomFlags*>(&what)));
            if (isDebug) {
                puts("AnyOptCustomFlags moved data");
                fflush(stdout);
            }
        } else {
            if (isDebug) {
                puts("AnyOptCustomFlags allocating and assigning data");
                fflush(stdout);
            }
            deallocate();
            const_cast<AnyOptCustomFlags*>(this)->data = new storage<typename std::remove_reference<T>::type>(std::move(what), isDebug);
            if (isDebug) {
                puts("AnyOptCustomFlags allocated and assigned data");
                fflush(stdout);
            }
            const_cast<AnyOptCustomFlags*>(this)->isAnyNullOpt = false;
            const_cast<AnyOptCustomFlags*>(this)->data_is_allocated = true;
        }
    }

    template<typename T>
    void store_pointer(T * what, bool allocated, const char * type) const {
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_ENABLE_POINTERS,
                "this function is not allowed unless pointers are enabled"
        );
        if (isDebug) {
            printf("AnyOptCustomFlags pointer %s\n", type);
            fflush(stdout);
        }

        // TODO: should pointers be reallocated if they are synthetic (allocated == false)?

        deallocate();
        if (isDebug) {
            puts("AnyOptCustomFlags allocating and assigning data");
            fflush(stdout);
        }
        const_cast<AnyOptCustomFlags*>(this)->data = new storage<T>(what, allocated, isDebug);
        const_cast<AnyOptCustomFlags*>(this)->data_is_allocated = allocated;
        if (isDebug) {
            puts("AnyOptCustomFlags allocated and assigned data");
            fflush(stdout);
        }
        const_cast<AnyOptCustomFlags*>(this)->isAnyNullOpt = false;
    }

    template<class T, typename = typename std::enable_if<!std::is_same<typename std::remove_reference<T>::type, void>::value, T>::type>
    AnyOptCustomFlags(const T &what) {
        if (flag_is_set(FLAGS, AnyOpt_FLAG_DEBUG)) isDebug = true;
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_COPY_ONLY,
                "this constructor is not allowed unless the copy flag is set"
        );
        store_copy(what, "constructor");
    }

    template<class T, typename = typename std::enable_if<!std::is_same<typename std::remove_reference<T>::type, void>::value, T>::type>
    AnyOptCustomFlags(T &&what) {
        if (flag_is_set(FLAGS, AnyOpt_FLAG_DEBUG)) isDebug = true;
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_MOVE_ONLY,
                "this constructor is not allowed unless the move flag is set"
        );
        store_move(std::move(what), "constructor");
    }

    template<typename T>
    AnyOptCustomFlags(T * what) {
        if (flag_is_set(FLAGS, AnyOpt_FLAG_DEBUG)) isDebug = true;
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_ENABLE_POINTERS,
                "this constructor is not allowed unless pointers are enabled is set"
        );
        store_pointer(what, false, "constructor");
    }

    template<typename T>
    AnyOptCustomFlags(T * what, bool allocation) {
        if (flag_is_set(FLAGS, AnyOpt_FLAG_DEBUG)) isDebug = true;
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_ENABLE_POINTERS,
                "this constructor is not allowed unless pointers are enabled is set"
        );
        store_pointer(what, allocation, "constructor");
    }

    AnyOptCustomFlags(const AnyOptCustomFlags &what) {
        if (flag_is_set(FLAGS, AnyOpt_FLAG_DEBUG)) isDebug = true;
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_COPY_ONLY,
                "this constructor is not allowed unless the copy flag is set"
        );
        store_copy(what, "constructor");
    }

    AnyOptCustomFlags(AnyOptCustomFlags &&what) {
        if (flag_is_set(FLAGS, AnyOpt_FLAG_DEBUG)) isDebug = true;
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_MOVE_ONLY,
                "this constructor is not allowed unless the move flag is set"
        );
        if (isDebug) {
            puts("AnyOptCustomFlags move constructor");
            fflush(stdout);
        }
        move(&what);
    }

    // this MUST be able to be called regardless of stored type
    void deallocate() const {
        if (isDebug) {
            puts("AnyOptCustomFlags deallocating data");
            fflush(stdout);
        }
        if (data != nullptr) {
            if (isDebug) {
                puts("AnyOptCustomFlags data is not nullptr");
                puts("AnyOptCustomFlags deleting data");
                fflush(stdout);
            }
            delete data;
            const_cast<AnyOptCustomFlags*>(this)->data = nullptr;
            const_cast<AnyOptCustomFlags*>(this)->data_is_allocated = false;
            if (isDebug) {
                puts("AnyOptCustomFlags deleted data");
                fflush(stdout);
            }
        } else {
            if (isDebug) {
                puts("AnyOptCustomFlags data is nullptr, data has not been allocated or has already been deallocated");
                fflush(stdout);
            }
        }
        const_cast<AnyOptCustomFlags*>(this)->isAnyNullOpt = true;
    }

    AnyOptCustomFlags &operator=(const AnyOptCustomFlags & what)  {
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_COPY_ONLY,
                "this assignment operator is not allowed unless the copy flag is set"
        );
        if ((void*)this != (void*)&what) store_copy(what, "assignment");
        return *const_cast<AnyOptCustomFlags*>(this);
    }

    AnyOptCustomFlags &operator=(const AnyNullOpt_t & what)  {
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_COPY_ONLY,
                "this assignment operator is not allowed unless the copy flag is set"
        );
        if (isDebug) {
            puts("AnyOptCustomFlags AnyNullOpt_t copy assignment");
            fflush(stdout);
        }
        deallocate();
        return *const_cast<AnyOptCustomFlags*>(this);
    }

    AnyOptCustomFlags &operator=(AnyNullOpt_t && what)  {
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_MOVE_ONLY,
                "this assignment operator is not allowed unless the move flag is set"
        );
        if (isDebug) {
            puts("AnyOptCustomFlags AnyNullOpt_t move assignment");
            fflush(stdout);
        }
        deallocate();
        return *const_cast<AnyOptCustomFlags*>(this);
    }

    AnyOptCustomFlags &operator=(AnyNullOpt_t ** what)  {
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_ENABLE_POINTERS,
                "this assignment operator is not allowed unless pointers are enabled"
        );
        if (isDebug) {
            puts("AnyOptCustomFlags AnyNullOpt_t move assignment");
            fflush(stdout);
        }
        deallocate();
        return *const_cast<AnyOptCustomFlags*>(this);
    }

    AnyOptCustomFlags &operator=(const nullptr_t & what)  {
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_COPY_ONLY,
                "this assignment operator is not allowed unless the copy flag is set"
        );
        if (isDebug) {
            puts("AnyOptCustomFlags nullptr_t copy assignment");
            fflush(stdout);
        }
        deallocate();
        return *const_cast<AnyOptCustomFlags*>(this);
    }

    AnyOptCustomFlags &operator=(nullptr_t && what)  {
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_MOVE_ONLY,
                "this assignment operator is not allowed unless the move flag is set"
        );
        if (isDebug) {
            puts("AnyOptCustomFlags nullptr_t move assignment");
            fflush(stdout);
        }
        deallocate();
        return *const_cast<AnyOptCustomFlags*>(this);
    }

    template<class T, typename = typename std::enable_if<!std::is_same<typename std::remove_reference<T>::type, void>::value, T>::type>
    AnyOptCustomFlags &operator=(const T & what)  {
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_COPY_ONLY,
                "this assignment operator is not allowed unless the copy flag is set"
        );
        if ((void*)this != (void*)&what) store_copy(what, "assignment");
        return *const_cast<AnyOptCustomFlags*>(this);
    }

    template<class T, typename = typename std::enable_if<!std::is_same<typename std::remove_reference<T>::type, void>::value, T>::type>
    AnyOptCustomFlags &operator=(T &&what)  {
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_MOVE_ONLY,
                "this assignment operator is not allowed unless the move flag is set"
        );
        if ((void*)this != (void*)&what) store_move(what, "assignment");
        return *const_cast<AnyOptCustomFlags*>(this);
    }

    template<typename T> AnyOptCustomFlags &operator=(T * what)  {
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_ENABLE_POINTERS,
                "this assignment operator is not allowed unless pointers are enabled"
        );
        if ((void*)this != (void*)&what) store_pointer(what, false, "assignment");
        return *const_cast<AnyOptCustomFlags*>(this);
    }

    AnyOptCustomFlags &store(const AnyNullOpt_t & what) const {
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_COPY_ONLY,
                "this function is not allowed unless the copy flag is set"
        );
        if (isDebug) {
            puts("AnyOptCustomFlags AnyNullOpt_t copy store");
            fflush(stdout);
        }
        deallocate();
        return *const_cast<AnyOptCustomFlags*>(this);
    }

    AnyOptCustomFlags &store(AnyNullOpt_t && what) const {
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_MOVE_ONLY,
                "this function is not allowed unless the move flag is set"
        );
        if (isDebug) {
            puts("AnyOptCustomFlags AnyNullOpt_t move store");
            fflush(stdout);
        }
        deallocate();
        return *const_cast<AnyOptCustomFlags*>(this);
    }

    template<class T, typename = typename std::enable_if<!std::is_same<typename std::remove_reference<T>::type, void>::value, T>::type>
    AnyOptCustomFlags &store(const T & what) const {
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_COPY_ONLY,
                "this function is not allowed unless the copy flag is set"
        );
        store_copy(what, "assignment");
        return *const_cast<AnyOptCustomFlags*>(this);
    }

    template<class T, typename = typename std::enable_if<!std::is_same<typename std::remove_reference<T>::type, void>::value, T>::type>
    AnyOptCustomFlags &store(T && what) const {
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_MOVE_ONLY,
                "this function is not allowed unless the move flag is set"
        );
        // function accepting a forward reference
        store_move(what, "assignment");
        return *const_cast<AnyOptCustomFlags*>(this);
    }

    AnyOptCustomFlags &store(const AnyNullOpt_t & what, bool allocated) const {
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_ENABLE_POINTERS,
                "this function is not allowed unless pointers are enabled"
        );
        deallocate();
        return *const_cast<AnyOptCustomFlags*>(this);
    }

    template<typename T> AnyOptCustomFlags &store(T * what, bool allocated) const {
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_ENABLE_POINTERS,
                "this function is not allowed unless pointers are enabled"
        );
        store_pointer(what, allocated, "assignment");
        return *const_cast<AnyOptCustomFlags*>(this);
    }


//    bool compare(const AnyOptCustomFlags &lhs, const AnyOptCustomFlags &rhs) const {
// TYPES ARE REQUIRED TO BE KNOWN
//        puts("AnyOptCustomFlags comparison");
//        fflush(stdout);
//        return
//            (lhs.data == rhs.data) &&
//            (lhs.isAnyNullOpt == rhs.isAnyNullOpt) &&
//            (lhs.data_is_allocated == rhs.data_is_allocated);
//    }
//
//    bool operator!=(const AnyOptCustomFlags &rhs) const {
// TYPES ARE REQUIRED TO BE KNOWN
//        return !compare(*this, rhs);
//    }
//
//    bool operator==(const AnyOptCustomFlags &rhs) const {
// TYPES ARE REQUIRED TO BE KNOWN
//        return compare(*this, rhs);
//    }
//
    bool operator==(const AnyNullOpt_t &rhs) const {
        if (isDebug) {
            puts("AnyOptCustomFlags nullptr_t compare ==");
            fflush(stdout);
        }
        return data == nullptr && data_is_allocated == false && isAnyNullOpt == true;
    }

    bool operator==(const nullptr_t &rhs) const {
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_ENABLE_POINTERS,
                "this function is not allowed unless pointers are enabled"
        );
        if (isDebug) {
            puts("AnyOptCustomFlags nullptr_t compare ==");
            fflush(stdout);
        }
        return data == nullptr && data_is_allocated == false;
    }

    bool operator!=(const AnyNullOpt_t &rhs) const {
        if (isDebug) {
            puts("AnyOptCustomFlags nullptr_t compare !=");
            fflush(stdout);
        }
        return data != nullptr && isAnyNullOpt == false;
    }

    bool operator!=(const nullptr_t &rhs) const {
        ensure_flag_enabled(
                FLAGS,
                AnyOpt_FLAG_ENABLE_POINTERS,
                "this function is not allowed unless pointers are enabled"
        );
        if (isDebug) {
            puts("AnyOptCustomFlags nullptr_t compare !=");
            fflush(stdout);
        }
        return data != nullptr;
    }
    AnyOptCustomFlags() {
        if (flag_is_set(FLAGS, AnyOpt_FLAG_DEBUG)) isDebug = true;
        if (isDebug) {
            puts("AnyOptCustomFlags constructor");
            fflush(stdout);
        }
    }

    ~AnyOptCustomFlags() {
        if (isDebug) {
            puts("AnyOptCustomFlags destructor");
            fflush(stdout);
        }
        deallocate();
    }

    template<class T = void, typename = typename std::enable_if<std::is_pointer<T>::value, T>::type>
    T get_impl(T * unused1 = nullptr, T * unused2 = nullptr) const {
        if (data == nullptr) {
            if (isDebug) {
                RUNTIME_WARNING_(
                        "trying to obtain data when no data is stored,"
                        " nullptr will be returned instead"
                );
            }
            return nullptr;
        }
        storage<typename std::remove_pointer<T>::type>* s = static_cast<storage<typename std::remove_pointer<T>::type>*>(data);
        return s->data;
    }
    
    template<class T = void, typename = typename std::enable_if<!std::is_pointer<T>::value, T>::type>
    T get_impl(T * unused1 = nullptr) const {
        RUNTIME_ASSERTION(data != nullptr, "trying to obtain data when no data is stored");
        storage<typename std::remove_pointer<T>::type>* s = static_cast<storage<typename std::remove_pointer<T>::type>*>(data);
        return *s->data;
    }

    template<class T = void, typename = typename std::enable_if<!std::is_pointer<T>::value && std::is_lvalue_reference<T>::value, T>::type>
    T get_impl() const {
        RUNTIME_ASSERTION(data != nullptr, "trying to obtain data when no data is stored");
        storage<typename std::remove_reference<typename std::remove_pointer<T>::type>::type>* s = static_cast<storage<typename std::remove_reference<typename std::remove_pointer<T>::type>::type>*>(data);
        return *s->data;
    }
    
    template <typename T> T get() const {
        if (isDebug) {
            puts("AnyOptCustomFlags get");
            fflush(stdout);
        }
        return get_impl<T>();
    }
};

typedef AnyOptCustomFlags<AnyOpt_FLAGS_DEFAULT> AnyOpt;

typedef AnyOptCustomFlags<AnyOpt_FLAGS_DEFAULT|AnyOpt_FLAG_DEBUG> AnyOptDebug;

template <typename T> const T AnyOpt_Cast(const AnyOpt & val)  {
    return val.get<T>();
}

#endif //AnyOpt_H
