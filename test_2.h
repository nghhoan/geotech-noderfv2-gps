#ifndef TEST_2_H
#define TEST_2_H

#define max_fields 20

// Khai báo prototype các hàm xử lý NMEA
int isValidNMEA(char *buffer);
int isChecksumInString(char *buffer);
char *AddressOfCheckSumSign(char *buffer);
void getChecksum(char *buffer, char checksum[3]);
int isLengValid(char *buffer);
uint8_t calChecksum(char *line);
int parsing(char line[], char *field[]);
void timeProcessing(char* field, uint8_t *hour, uint8_t *minute, uint8_t *second);
void latProcessing(char* field_toado, char* dir,double *toado, uint16_t *dd, double *min);
void lonProcessing(char* field_toado, char* dir,double *toado, uint16_t *dd, double *min);

#endif