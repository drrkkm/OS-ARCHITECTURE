#include <unistd.h>

int main() {
	sleep(0.1);
	write(1, "I FINISHED\n", 11);
	return 0;
}
