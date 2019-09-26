#include <stdio.h>
int main (int argc, char **argv){
  printf("You started driving. Now you are driving straight.\n");
  printf("Please press 'l' if you want to go left, or 'r' if you want to go right.\n");

while(1){
	char c = getchar();
  if (c == 'l') {
  	printf("Turning left. \n");
  	printf("2 seconds passed. You have made your turn and countinues to drive straight.\n");
	}
  else if (c == 'r'){
  	printf("Turning right. \n");
  	printf("2 seconds passed. You have made your turn and countinues to drive straight.\n");

  }
  }
  return 0;
}