#include "threadpool.h"

int main(int argc, char **argv)
{
	ThreadPool pool;
	pool.init(10);
	return 0;
}