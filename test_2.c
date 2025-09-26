#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "test_2.h"

/*
$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
- Field[0]: $GPRMC => GPRMC có dạng time, status (A/V), lat, N/S, lon, E/W, date
    còn GPGGA có dạng time, lat, N/S, lon, E/W, fix quality, num satellites, HDOP, altitude
    => phải kiểm tra số lượng field hoặc ',' ứng với dạng sentence.
- Field[1]: 123519 ==> phải đổi thành dạng hh:mm:ss
- Field[2]: A ==> Active
- Field[3]: 4807.038 ==> chỉ số lat có dạng ddmm.mmmm. phải chuyển sang lat_decimal = dd(int) + (mm.mmmm/60)(float)
- Field[4]: N ==> North/South ==> S thì lat_decimal = -lat_decimal, North thì lat_decimal giữ nguyên
- Field[5]: 01131.000 =>> lon: ddmm.mmmm => lon_decimal : dd(int) + (mm.mmmm/60)(float)
- Field[6]: E ==> East/West ==> W thì lon_decimal = -lon_decimal, E thì giữ nguyên.
- Field[7]: 022.4 
- Field[8]: 084.4
- Field[9]: 230394
- Field[10]: 003.1
- Field[11]: W*6A ==> checksum = 6A. Tính XOR các kí tự từ sau dấu $ đến trước dấu *, nếu = 6A thì checksum hợp lệ.
*/

//hàm chech xem chuỗi nhập vào hợp lệ không
int isValidNMEA(char *buffer){
    char *ptr = strchr(buffer, '$');
    if(ptr != NULL){
        return 1;
    }
    else return 0;
}

//hàm check checksum hợp lệ không
int isChecksumInString(char *buffer){
    char *ptr;
    ptr = strchr(buffer, '*');
    if(ptr != NULL){
        return 1;
    }
    else return 0;
}
//trả về địa chỉ dấu checksum
char *AddressOfCheckSumSign(char *buffer){
    char *ptr;
    ptr = strchr(buffer, '*'); 
    if(ptr != NULL){
        return ptr;
    }
    return NULL;
}
//Hàm trả về checksum
void getChecksum(char *buffer, char checksum[3]){
    char *ptr;
    ptr = strchr(buffer, '*');
    if (!ptr || ptr[1] == '\0' || ptr[2] == '\0') { // không đủ 2 ký tự hex
        checksum[0] = '\0'; 
        return;
    }
    strncpy(checksum, ptr+ 1, 2);
    checksum[2] = '\0';
}

//hàm check số kí tự có vượt quá 82 không
int isLengValid(char *buffer){
    if(strlen(buffer) <= 82){
        return 1;
    }
    else return 0;
}


//hàm tính checksum từ chuỗi
uint8_t calChecksum(char *line) {
    uint8_t sum = 0;
    char *p = strchr(line, '$');
    if (!p) return 0;
    p++; // bắt đầu ngay sau '$'
    while (*p != '\0' && *p != '*') {
        sum ^= (uint8_t)(*p);
        p++;
    }
    return sum;
    
}
//validate
static int validateTime(uint8_t h, uint8_t m, uint8_t s){
    return (h < 24 && m < 60 && s < 60);
}
static int validateLat(uint16_t deg, double minutes, double val){
    if (deg > 90) return 0; if (minutes < 0 || minutes >= 60) return 0; if (val < -90.0 || val > 90.0) return 0; return 1; }
static int validateLon(uint16_t deg, double minutes, double val){
    if (deg > 180) return 0; if (minutes < 0 || minutes >= 60) return 0; if (val < -180.0 || val > 180.0) return 0; return 1; }

//xử lí tách các field ra
int parsing(char line[], char *field[]){
    int count = 0;
    char *p = line;
    field[count++] = p;

    while (*p != '\0' && count < max_fields)
    {
        if(*p == ','){
            *p = 0;
            field[count++] = p+1;
        }
        p++;
    }
    return count;
}

//xử lí field time
void timeProcessing(char* field, uint8_t *hour, uint8_t *minute, uint8_t *second){
    *hour = *minute = *second = 0;
    if(!field || strlen(field) < 6){
        printf("Time field too short\n");
        return;
    }
    if (sscanf(field,"%2hhu%2hhu%2hhu",hour,minute,second) != 3){
        printf("Time parse fail\n");
        return;
    }
    if(!validateTime(*hour,*minute,*second)){
        printf("Time out of range (%u:%u:%u)\n", *hour,*minute,*second);
        return;
    }
    printf("Thoi gian da parsing la: %u:%u:%u\n",*hour,*minute,*second);
}


//xử lí field toạ độ lat
void latProcessing(char* field_toado, char* dir,double *toado, uint16_t *dd, double *min){
    *toado = 0.0; *dd = 0; *min = 0.0;
    if(!field_toado || !dir) return;
    if (sscanf(field_toado, "%2hu%lf", dd, min) != 2){
        printf("Lat parse fail\n"); return; }
    *toado = *dd + (*min/60.0);
    if (*dir == 'S') *toado = -*toado; // N giữ nguyên
    if(!validateLat(*dd, *min, *toado)){
        printf("Lat out of range (deg=%u, min=%f, val=%f)\n", *dd, *min, *toado);
    }
}


//xử lí field toạ độ lon
void lonProcessing(char* field_toado, char* dir,double *toado, uint16_t *dd, double *min){
    *toado = 0.0; *dd = 0; *min = 0.0;
    if(!field_toado || !dir) return;
    if (sscanf(field_toado, "%3hu%lf", dd, min) != 2){
        printf("Lon parse fail\n"); return; }
    *toado = *dd + (*min/60.0);
    if (*dir == 'W') *toado = -*toado; // E giữ nguyên
    if(!validateLon(*dd, *min, *toado)){
        printf("Lon out of range (deg=%u, min=%f, val=%f)\n", *dd, *min, *toado);
    }
}




int main(){
    int fcount;
    char checksum[3];
    char line[100];
    char *field[max_fields];
    printf("Nhap vao chuoi: ");
    fgets(line, sizeof(line), stdin);
    line[strcspn(line, "\r\n")] = '\0';

    if(line[0] == '$' && isLengValid(line)){
        printf("Chuoi hop le!\n");

        if(isChecksumInString(line)){
            getChecksum(line, checksum);
            if(checksum[0] == '\0'){
                printf("Checksum khong hop le!");
            } else {
                uint8_t checksum_val = (uint8_t) strtoul(checksum,NULL,16);
                printf("Checksum trong chuoi = %02X\n",checksum_val);
                printf("Checksum tinh toan = %02X\n", calChecksum(line));
                if(calChecksum(line) == checksum_val){
                    printf("=> Checksum hop le!\n");
                } else {
                    printf("=> Checksum khong hop le\n");
                }
            }
        } else printf("=> Checksum khong hop le!\n");
    }
    uint8_t hour,minute,second;
    double lat,lon;
    uint16_t lat_dec, lon_dec;
    double lat_min, lon_min;
    fcount = parsing(line, field);
    if(fcount >= 7 && strcmp(field[2], "A") == 0){
        printf("=> Du lieu hop le!\n");
        for(int i = 0; i < fcount; i++){
            printf("Field[%d]: %s\n",i,field[i]);
        }
        timeProcessing(field[1], &hour, &minute, &second);
        latProcessing(field[3], field[4], &lat, &lat_dec, &lat_min);
        lonProcessing(field[5], field[6], &lon, &lon_dec, &lon_min);
        if(validateTime(hour,minute,second) && lat >= -90 && lat <= 90 && lon >= -180 && lon <= 180){
            printf("Toa do lat da parsing la: %lf\n", lat);
            printf("Toa do lon da parsing la: %lf\n", lon);
        } else {
            printf("=> Du lieu bi loai do sai mien\n");
        }
    } else {
        printf("=> Chuoi khong hop le!\n");
    }
}

