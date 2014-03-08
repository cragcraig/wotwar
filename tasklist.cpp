#include "wotwar.h"

// TASKLIST class functions

void TASKLIST::add(TASK* task) {
	if (pFirst != NULL) pLast->pNext = task;
	else pFirst = task;
	pLast = task;
	pLast->pNext = NULL;
	length++;
}

// insert task after link
TASK* TASKLIST::insert(TASK* link, TASK* task) {
	if (!pFirst) {
		pFirst = task;
		task->pNext = NULL;
	}
	if (link) {
		task->pNext = link->pNext;
		link->pNext = task;
	}
	else {
		task->pNext = pFirst;
		pFirst = task;
	}
	if (pLast == link) pLast = task;
	length++;
	return task;
}

// insert a list of tasks after link
TASK* TASKLIST::insertList(TASK* link, TASK* task) {
	bool replace_next = task->replace_next;
	TASK* last, *cur, *tmp;
	if (!pFirst) {
		pFirst = task;
		last = NULL;
	}
	if (link) {
		last = link->pNext;
		link->pNext = task;
	}
	else {
		last = pFirst;
		pFirst = task;
	}
	length++;
	cur = task;
	while (cur->pNext) { // loop though all added tasks
		length++;
		cur = cur->pNext;
	}
	cur->pNext = last;
	if (pLast == link) pLast = cur;
	// delete next
	if (replace_next && pLast != cur) {
		tmp = cur->pNext;
		cur->pNext = tmp->pNext;
		if (!cur->pNext) pLast = cur;
		cur->is_pathfind = tmp->is_pathfind;
		delete tmp;
		length--;
	}
	return cur;
}

TASK* TASKLIST::add_front(TASK* task)
{
	if (!pLast) pLast = task;
	task->pNext = pFirst;
	pFirst = task;
	length++;
	return task;
}

GROUP* TASKLIST::find_next_attack(void)
{
    if (!pFirst) return NULL;
    TASK* t = pFirst;
    do {
        if (t->type == TASK::FIGHT) return t->link;
    } while (t = t->pNext);
    return NULL;
}

void TASKLIST::drop_until_attack(void)
{
    if (!pFirst) return;
    TASK* t = pFirst;
    do {
        if (t->type == TASK::FIGHT) return;
        else endCurrent();
    } while (t = pFirst);
    return;
}

void TASKLIST::endCurrent(void) {
	if (pFirst != NULL) {
		length--;
		TASK* tmpPointer = pFirst->pNext;
		if (pFirst->type == TASK::WAIT_SIG) pAH->setTaskNextLoop(new TASK(TASK::CHECK_WAIT, pFirst->player, pFirst->id)); // if destroying a WAIT_SIG send signal to group that agent is in position
		delete pFirst;
		pFirst = tmpPointer;
		if (pFirst == NULL) pLast = NULL;
	}
}

void TASKLIST::drop_path_tasks(void)
{
	TASK* tmp;
	while (pFirst && pFirst->is_pathfind) {
		length--;
		tmp = pFirst->pNext;
		delete pFirst;
		pFirst = tmp;
		if (pFirst == NULL) pLast = NULL;
	}
	if (pFirst) pFirst->pathfind_level--;
}

// count builds
void TASKLIST::count_builds(int ret_array[])
{
    TASK* tmp;
    for (int i=0; i<MAX_BUILDS; i++) ret_array[i] = 0;
    if (!pFirst) return;
    TASK* t = pFirst;
    do {
        if (t->type == TASK::BUILD) ret_array[t->build_id]++;
    } while (t = t->pNext);
}

void TASKLIST::dropAll(void) {
	while (length) endCurrent();
}

TASKLIST::TASKLIST(void) {
	pLast = pFirst = NULL;
	length = 0;
}

TASKLIST::~TASKLIST(void) {
	dropAll();
}
