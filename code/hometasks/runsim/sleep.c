#include <unistd.h>

int main() {
	sleep(10);
	write(1, "I FINISHED", 11);
	return 0;
}
