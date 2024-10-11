#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <global.hpp>

#define PWM_CHIP_PATH "/sys/class/pwm/pwmchip"

int pwm_export(int channel_num)
{
    char export_path[64];
    snprintf(export_path, sizeof(export_path), "%s%d/export", PWM_CHIP_PATH, channel_num);

    int export_fd = open(export_path, O_WRONLY);
    if (export_fd < 0) {
        // fprintf(stderr, "%d: export_path=[%s] fail.\n", __LINE__, export_path);
        // perror("Failed to open PWM export file");
        xlog("%s:%d, open PWM export fail, path:%s \n\r", __func__, __LINE__, export_path);
        return -1;
    }

    char channel_str[2];
    snprintf(channel_str, sizeof(channel_str), "0");
    ssize_t num_written = write(export_fd, channel_str, sizeof(channel_str));
    if (num_written < 0) {
        // fprintf(stderr, "%d: export_path=[%s] channel_str=[%s] fail.\n", __LINE__, export_path, channel_str);
        // perror("Failed to write to PWM export file");
        xlog("%s:%d, write PWM export fail, path:%s, channel:%s \n\r", __func__, __LINE__, export_path, channel_str);
        return -1;
    }

    close(export_fd);

    // printf("PWM channel %d has been exported\n", channel_num);
    return 0;
}

int pwm_write_period(int channel_num, int period)
{
    char period_path[64];
    snprintf(period_path, sizeof(period_path), "%s%d/pwm0/period", PWM_CHIP_PATH, channel_num);

    int period_fd = open(period_path, O_WRONLY);
    if (period_fd < 0) {
        fprintf(stderr, "%d: period_path=[%s] fail.\n", __LINE__, period_path);
        perror("Failed to open PWM period file");
        return -1;
    }

    char period_str[16];
    snprintf(period_str, sizeof(period_str), "%d", period);
    ssize_t num_written = write(period_fd, period_str, sizeof(period_str));
    if (num_written < 0) {
        fprintf(stderr, "%d: period_path=[%s] period_str=[%s] fail.\n", __LINE__, period_path, period_str);
        perror("Failed to write to PWM period file");
        return -1;
    }

    close(period_fd);

    printf("PWM channel %d period set to %d\n", channel_num, period);

    return 0;
}

int pwm_write_duty_cycle(int channel_num, int duty_cycle)
{
    char duty_cycle_path[64];
    snprintf(duty_cycle_path, sizeof(duty_cycle_path), "%s%d/pwm0/duty_cycle", PWM_CHIP_PATH, channel_num);

    int duty_cycle_fd = open(duty_cycle_path, O_WRONLY);
    if (duty_cycle_fd < 0) {
        perror("Failed to open PWM duty cycle file");
        fprintf(stderr, "%d: duty_cycle_path=[%s] fail.\n", __LINE__, duty_cycle_path);
        return -1;
    }

    char duty_cycle_str[16];
    snprintf(duty_cycle_str, sizeof(duty_cycle_str), "%d", duty_cycle);
    ssize_t num_written = write(duty_cycle_fd, duty_cycle_str, sizeof(duty_cycle_str));
    if (num_written < 0) {
        perror("Failed to write to PWM duty cycle file");
        fprintf(stderr, "%d: duty_cycle_str=[%s] fail.\n", __LINE__, duty_cycle_str);
        return -1;
    }

    close(duty_cycle_fd);

    //printf("PWM channel %d duty cycle set to %d\n", channel_num, duty_cycle);

    return 0;
}

int pwm_read_duty_cycle(int channel_num)
{
    char duty_cycle_path[64];
    snprintf(duty_cycle_path, sizeof(duty_cycle_path), "%s%d/pwm0/duty_cycle", PWM_CHIP_PATH, channel_num);

    FILE *duty_cycle_fd = fopen(duty_cycle_path, "r");
    if (duty_cycle_fd == NULL) {
        perror("Failed to open PWM duty cycle file");
        return -1;
    }

    char duty_cycle_str[16];
    // fgets(&duty_cycle_str[0], sizeof(duty_cycle_str), duty_cycle_fd);
    if (fgets(duty_cycle_str, sizeof(duty_cycle_str), duty_cycle_fd) == NULL) {
      // Handle the error, for example by printing an error message
      printf("Error reading duty cycle from file");
      fclose(duty_cycle_fd);
      return -1;
    }

    fclose(duty_cycle_fd);
    //printf("PWM channel %d duty cycle set to %d\n", channel_num, duty_cycle);

    return strtol(&duty_cycle_str[0], NULL, 10);
}

int pwm_write_polarity(int channel_num, char *polarity)
{
    char polarity_path[64];
    snprintf(polarity_path, sizeof(polarity_path), "%s%d/pwm0/polarity", PWM_CHIP_PATH, channel_num);

    int polarity_fd = open(polarity_path, O_WRONLY);
    if (polarity_fd < 0) {
        perror("Failed to open PWM polarity file");
        return -1;
    }

    char polarity_str[16];
    snprintf(polarity_str, sizeof(polarity_str), "%s", polarity);
    ssize_t num_written = write(polarity_fd, polarity_str, sizeof(polarity_str));
    if (num_written < 0) {
        perror("Failed to write to PWM polarity file");
        return -1;
    }

    close(polarity_fd);

    printf("PWM channel %d polarity set to %s\n", channel_num, polarity);

    return 0;
}

int pwm_write_enable(int channel_num, int enable)
{
    char enable_path[64];
    snprintf(enable_path, sizeof(enable_path), "%s%d/pwm0/enable", PWM_CHIP_PATH, channel_num);

    int enable_fd = open(enable_path, O_WRONLY);
    if (enable_fd < 0) {
        //perror("Failed to open PWM enable file");
        return -1;
    }

    char enable_str[16];
    snprintf(enable_str, sizeof(enable_str), "%d", enable);
    ssize_t num_written = write(enable_fd, enable_str, sizeof(enable_str));
    if (num_written < 0) {
        perror("Failed to write to PWM enable file");
        return -1;
    }

    close(enable_fd);

    printf("PWM channel %d enable set to %d\n", channel_num, enable);

    return 0;
}

int pwm_unexport(int channel_num)
{
    char unexport_path[64];
    snprintf(unexport_path, sizeof(unexport_path), "%s%d/unexport", PWM_CHIP_PATH, channel_num);
    
    int export_fd = open(unexport_path, O_WRONLY);
    if (export_fd < 0) {
        perror("Failed to open PWM unexport file");
        return -1;
    }

    char channel_str[2];
    snprintf(channel_str, sizeof(channel_str), "%d", channel_num);
    ssize_t num_written = write(export_fd, channel_str, sizeof(channel_str));
    if (num_written < 0) {
        perror("Failed to write to PWM unexport file");
        return -1;
    }

    close(export_fd);

    printf("PWM channel %d has been unexport\n", channel_num);

    return 0;
}
