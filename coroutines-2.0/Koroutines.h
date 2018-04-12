
#ifndef KOROUTINE_H
#define KOROUTINE_H

struct Koroutine;
typedef void* (*KoroutineFunc)(void*);

//void Init();
//void Release();

Koroutine* Create( KoroutineFunc f, size_t stacksize, void* arg );
void Delete( Koroutine* &k );

void* Yield( void* arg );
void* Resume( Koroutine* k, void* arg );
bool IsRunning( Koroutine* k );

#endif

