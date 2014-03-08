#ifndef LINK_H
#define LINK_H

#include "wotwar_classes.h"

// Class T must provide methods link(LINK<T>*) and unlink(LINK<T>*) and must call unlink on all currently linked LINKs in its destructor.
template<typename T>
class LINK
{
	public:
		LINK(void);
		LINK(T * p);
		~LINK(void);
		void unlink(void);
		void kill_link(void);
		void switch_link(T * p);
		T* get_link(void);
		bool is_dead(void);

	private:
		T* l;
};

template<typename T>
LINK<T>::LINK(void) : l(NULL)
{
}

template<typename T>
LINK<T>::LINK(T * p) : l(p)
{
	if (l) l->link(this);
}

template<typename T>
LINK<T>::~LINK(void)
{
	unlink();
}

template<typename T>
T* LINK<T>::get_link(void)
{
	return l;
}

template<typename T>
bool LINK<T>::is_dead(void)
{
	return (l == NULL);
}

template<typename T>
void LINK<T>::unlink(void)
{
	if (l) {
		l->unlink(this);
		l = NULL;
	}
}

template<typename T>
void LINK<T>::kill_link(void)
{
	if (l) l = NULL;
}

template<typename T>
void LINK<T>::switch_link(T * p)
{
	unlink();
	l = p;
	if (l) l->link(this);
}

#endif
