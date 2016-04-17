#ifndef _E_GLOBAL_MUTEX_H_INCLUDED
#define _E_GLOBAL_MUTEX_H_INCLUDED

typedef int  e_mutex_t;
typedef int  e_mutexattr_t;

//-- for user to initialize a mutex structure
#define MUTEX_NULL     (0)
#define MUTEXATTR_NULL (0)
#define MUTEXATTR_DEFAULT MUTEXATTR_NULL

void _e_global_mutex_init(unsigned row, unsigned col, e_mutex_t *mutex, e_mutexattr_t *attr);
void _e_global_mutex_lock(unsigned row, unsigned col, e_mutex_t *mutex);
void _e_global_mutex_unlock(unsigned row, unsigned col, e_mutex_t *mutex);

#endif // _E_GLOBAL_MUTEX_H_INCLUDED
