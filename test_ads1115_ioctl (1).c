#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdint.h>

// IOCTL request codes (aligned with ADS1115 MUX config)
// Single-ended input (AINx vs GND): MUX = 0x4–0x7
#define ADC1_R  _IOR('a', 1, int)   // AIN0 (MUX = 100)
#define ADC2_R  _IOR('a', 2, int)   // AIN1 (MUX = 101)
#define ADC3_R  _IOR('a', 3, int)   // AIN2 (MUX = 110)
#define ADC4_R  _IOR('a', 4, int)   // AIN3 (MUX = 111)

// Differential input modes (AINx – AINy): MUX = 0x0–0x3
#define DIFF_0_1  _IOR('a', 11, int)   // AIN0 – AIN1 (MUX = 000)
#define DIFF_0_3  _IOR('a', 12, int)   // AIN0 – AIN3 (MUX = 001)
#define DIFF_1_3  _IOR('a', 13, int)   // AIN1 – AIN3 (MUX = 010)
#define DIFF_2_3  _IOR('a', 14, int)   // AIN2 – AIN3 (MUX = 011)

void read_adc(int fd, unsigned long request, const char* label, float fsr) {
    int val;
    int16_t val_signed;
    float voltage;

    if (ioctl(fd, request, &val) < 0) {
        perror("ioctl failed");
        return;
    }

    val_signed = (int16_t)val;
    voltage = val_signed * (fsr / 32768.0f);

    // In theo định dạng bảng
    printf("| %-22s | %6d | %+9.4f V |\n", label, val_signed, voltage);
}


int main() {
    float fsr = 4.096f;  // Assuming PGA ±4.096V
    int fd = open("/dev/driver_ads1115", O_RDONLY);  // Match device name from driver

    if (fd < 0) {
        perror("Failed to open /dev/ads1115");
        return 1;
    }

    printf("===========================================\n");    
    printf("|             ADS1115 READINGS            |\n");
    printf("===========================================\n");

    printf("\n>> Single-Ended Inputs (AINx vs GND):\n");
    printf("+---------------------+--------+------------+\n");
    printf("| Channel             |  Raw   |  Voltage   |\n");
    printf("+---------------------+--------+------------+\n");
    read_adc(fd, ADC1_R, "AIN0 (Single)       ", fsr);
    read_adc(fd, ADC2_R, "AIN1 (Single)       ", fsr);
    read_adc(fd, ADC3_R, "AIN2 (Single)       ", fsr);
    read_adc(fd, ADC4_R, "AIN3 (Single)       ", fsr);
    printf("+---------------------+--------+------------+\n");

    printf("\n>> Differential Inputs (AINx - AINy):\n");
    printf("+------------------------+--------+------------+\n");
    printf("| Channel                |  Raw   |  Voltage   |\n");
    printf("+------------------------+--------+------------+\n");
    read_adc(fd, DIFF_0_1, "AIN0 - AIN1 (Diff)   ", fsr);
    read_adc(fd, DIFF_0_3, "AIN0 - AIN3 (Diff)   ", fsr);
    read_adc(fd, DIFF_1_3, "AIN1 - AIN3 (Diff)   ", fsr);
    read_adc(fd, DIFF_2_3, "AIN2 - AIN3 (Diff)   ", fsr);
    printf("+------------------------+--------+------------+\n");


    close(fd);
    return 0;
}
