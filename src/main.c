//-------| src/main.c |-------//
#include "main.h"


static off_t IEB_DOT[MAX_DOT] = {
   IEB_DOT1,
   IEB_DOT2,
   IEB_DOT3,
   IEB_DOT4,
   IEB_DOT5
};
static off_t IEB_FND[MAX_FND] = {
   IEB_FND0,
   IEB_FND1,
   IEB_FND2,
   IEB_FND3,
   IEB_FND4,
   IEB_FND5,
   IEB_FND6,
   IEB_FND7
};

static int fd;
static int map_counter = 0;
static void * map_data[100];
static seclection_t sel; 
static int floor = 1;
static int destination = 1;


int main(int argc, char* argv[]) {
   
   int i;
   int floor = 1; 
   short * led, * dot[MAX_DOT], * fnd[MAX_FND];
   short * clcd_cmd, * clcd_data, * keypad_out, * keypad_in;
   
   fd = open("/dev/mem", O_RDWR|O_SYNC);
   if (fd == -1) {
      fprintf(stderr, "Cannot open /dev/mem file");
      exit(EXIT_FAILURE);
   }
   
   led = mapper(IEB_LED, PROT_WRITE);
   for( i=0; i<MAX_DOT; i++ ) {
      dot[i] = mapper(IEB_DOT[i], PROT_WRITE);
   }
   for( i=0; i<MAX_FND; i++ ) {
      fnd[i] = mapper(IEB_FND[i], PROT_WRITE);
   }
   clcd_cmd  = mapper(IEB_CLCD_CMD, PROT_WRITE);
   clcd_data = mapper(IEB_CLCD_DATA, PROT_WRITE);
   keypad_out  = mapper(IEB_KEY_W, PROT_WRITE);
   keypad_in = mapper(IEB_KEY_R, PROT_READ);
   
   init_led(led);
   init_dot(dot);
   init_fnd(fnd);
   init_clcd(clcd_cmd, clcd_data);
   init_keypad(keypad_out, keypad_in);
   
   sel.all = 0; // notice only once
   while( logic() == TRUE ) {   } 
   
   unmapper();
   close(fd);
   return 0;
}

short * mapper(off_t offset, int prot) {
   map_data[map_counter] = mmap(NULL, sizeof(short), prot, MAP_SHARED, fd, offset);
   if ( map_data[map_counter] == MAP_FAILED ) {
      fprintf(stderr, "Cannot do mmap()");
      emergency_closer();
   }
   return (short *)map_data[map_counter++];
}

void unmapper() {
   int i;
   for( i=0; i<map_counter; i++) {
      munmap(map_data[i], sizeof(short));
   }
   map_counter = 0;
}

void emergency_closer() {
   unmapper();
   close(fd);
   exit(EXIT_FAILURE);
}

truth_t logic() {
   if( sel.all == 0 ) { 
	select_mode();
	sel.all = 1;  }
   else if( sel.exit == 1 ) { return FALSE; }
   else { input_mode(); }
   return TRUE;
}

void select_mode() {
   int i; 
   char buf[100];
   char clcd_start[] = "You are now 1F ";
   led_clear();
   dot_clear();
   fnd_clear();
   clcd_clear_display();
   clcd_write_string(clcd_start);
   printf("\n");
   printf("************** Elevator ************\n");
   printf("*   FND : Destination            *\n");
   printf("*   DOT : Your floor (Start : 1F)  *\n");
   printf("*   LED : Show 'up' or 'down'      *\n");
   printf("*   CLCD : Explain Situation       *\n");
   printf("*   Press '0' to exit elevator     *\n");
   printf("************************************\n\n");
   usleep(500000);	


}

void input_mode() {
   int key_value;
   char clcd_str[20];
   char go_up[] = "Going up";
   char go_down[] = "Going down ";
   char door_open[] = " same floor         ";
   char exit[] = " Good Bye";


   keypad_read(&key_value); // key_value = our input
   clcd_clear_display();

if (key_value < 10 && key_value > 0)  
{
   destination = key_value;

   /*led_bit(key_value);*/
   if (floor < destination) { // go up
      fnd_write(destination, 7);  // destination
   	while(destination != floor){
      		clcd_clear_display();
      		led_up_shift();
      		clcd_write_string(go_up);
      		floor++;
      		dot_write(floor);
      		usleep(200000);
      		dot_updown(1); // up sign
      		usleep(100000);
      	}
}

   else if (floor > destination) { // go down
      fnd_write(destination, 7);  // destination
      	while(destination != floor){
      		clcd_clear_display();
      		led_down_shift();
      		clcd_write_string(go_down);
      		floor--;
      		dot_write(floor);
      		usleep(200000);
      		dot_updown(2); // down sign
      		usleep(100000);
      	}
   }
   else {
	clcd_clear_display();
	clcd_write_string(door_open);
	usleep(500000);
	key_value = 11;
	}

}

else if (key_value == 0){
	 clcd_clear_display();
         clcd_write_string(exit);
	 sel.exit = 1;

}

else{
	dot_updown(3);
	clcd_clear_display();
	clcd_write_string(door_open);
	usleep(500000);
    }
}

//printf(" %d %d %d \n", floor, destination, key_value); // debug code, int number
