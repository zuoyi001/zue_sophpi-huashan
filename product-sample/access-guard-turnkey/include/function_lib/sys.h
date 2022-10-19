#ifndef SYS_H
#define SYS_H
int get_partition_total_size(const char* path);
int get_partition_available_size(const char* path);
void get_mac(char* card,char* mac_addr);
int get_time();
  int print_system_boot_time();


#endif 
