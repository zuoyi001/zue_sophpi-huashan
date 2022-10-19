#ifndef SYS_H
#define SYS_H

int get_partition_total_size(const char* path);
int get_partition_available_size(const char* path);
void get_mac(char* card,char* mac_addr);
int get_time();
int print_system_boot_time();
int get_system_boot_time(char* time);
void get_current_date(char *date);
void get_network_ip_address(char *interface, char *ip_address);
void get_network_netmask(char *interface, char *netmask);
void get_network_broadcast(char *interface, char *broadcast_address);

#endif
