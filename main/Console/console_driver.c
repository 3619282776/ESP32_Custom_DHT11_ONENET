#include "console_driver.h"
#include "string.h"
#include "stdint.h"
#include "esp_random.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "MQTT/mqtt_driver.h"
#include "esp_console.h"
#include "esp_heap_caps.h"
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"
#include "argtable3/argtable3.h"
static struct {
    struct arg_int *int_arg;
    struct arg_end *end;
}hello_cmd_args;
static struct{
    struct arg_str *str_arg;
    struct arg_end *end;
}suspend_args;
static struct{
    struct arg_str *str_arg;
    struct arg_end *end;
}resume_args;
static struct{
    struct arg_dbl *dbl_arg;
    struct arg_end* end;
}mqtt_args;
static int hello_world_cmd()//printf hello world without args
{
    printf("HELLO WORLD!!\n");
    return 0;
}
static int get_heap_cmd()//get heap
{
    size_t temp;
    temp = heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);
    printf("The most free heap is %u\n",temp);
    temp=heap_caps_get_free_size(MALLOC_CAP_8BIT);
    printf("Now free heap is %u\n",temp);
    return 0;
}
static int hello_world_arg_cmd(int argc,char **argv)// printf hello world with args
{
    int nerrors = arg_parse(argc, argv, (void**)&hello_cmd_args);
    if (nerrors != 0) {
        arg_print_errors(stdout, hello_cmd_args.end, "my_cmd");
        return 1;
    }
    int value = hello_cmd_args.int_arg->ival[0];
    printf("hello world ! and value is %d\n",value);
    return 0;
}
static int led_change_cmd()//change led level
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_NUM_2),
        .mode         = GPIO_MODE_INPUT_OUTPUT,
    };
    gpio_config(&io_conf);
    int level = gpio_get_level(GPIO_NUM_2);
    gpio_set_level(GPIO_NUM_2, !level);
    return 0;
}
static int get_task_num_state()//get running task and printf
{
    char pcWriteBuffer[1024];
    vTaskList(pcWriteBuffer);
    printf("Task list:\n%s\n", pcWriteBuffer);
    return 0;
}
static int supsend_task_cmd(int argc,char **argv)
{
    int nerrors = arg_parse(argc, argv, (void**)&suspend_args);
    if (nerrors != 0) {
        arg_print_errors(stdout, suspend_args.end, "my_cmd");
        return 1;
    }

    const char *str=suspend_args.str_arg->sval[0];
    TaskHandle_t handle=NULL;
    handle=xTaskGetHandle(str);
    if(handle==NULL)
    {printf("task not found\n");return 1;}
    else{
        vTaskSuspend(handle);
        printf("%s has been suspend\n",str);
        return 0;
    }
}
static int resume_task_cmd(int argc,char **argv)
{
    int nerrors = arg_parse(argc, argv, (void**)&resume_args);
    if (nerrors != 0) {
        arg_print_errors(stdout, resume_args.end, "my_cmd");
        return 1;
    }

    const char *str=resume_args.str_arg->sval[0];
    TaskHandle_t handle=NULL;
    handle=xTaskGetHandle(str);
    if(handle==NULL)
    {printf("task not found\n");return 1;}
    else{
        vTaskResume(handle);
        printf("%s has been resume\n",str);
        return 0;
    }
}
static int start_mqtt_cmd()
{
    printf("start begin mqtt\n");
    mqtt_init();
    return 0;
}

static int mqtt_cmd(int argc,char **argv)
{
    int nerrors = arg_parse(argc, argv, (void**)&mqtt_args);
    if (nerrors != 0) {
        arg_print_errors(stdout, mqtt_args.end, "my_cmd");
        return 1;
    }
    float tem=mqtt_args.dbl_arg->dval[0];
    printf("send %.1f to mqtt\n",tem);
    mqtt_send(tem);
    return 0;
}

static int random_cmd()
{
    uint32_t random =esp_random();
    random=random%100;
    printf("random number is %lu\n",random);
    float tem=random;
    printf("send %.1f to mqtt\n",tem);
    mqtt_send(tem);
    return 0;
}










void console_init()//init console
{
    //set repl
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.prompt="esp_command>>";
    //set urat
    esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    //make repl 
    esp_console_repl_t *repl = NULL;
    esp_console_new_repl_uart(&uart_config, &repl_config, &repl);
    //注册 command
    //设置参数cmd
    hello_cmd_args.int_arg=arg_int1(NULL, NULL, "<num>", "An integer parameter");
    hello_cmd_args.end = arg_end(1);

    suspend_args.str_arg=arg_str1(NULL, NULL, "<string>", "pleas input a task name to suspend");
    suspend_args.end = arg_end(1);

    resume_args.str_arg=arg_str1(NULL, NULL, "<string>", "pleas input a task name to resuem");
    resume_args.end = arg_end(1);

    mqtt_args.dbl_arg=arg_dbl1(NULL,NULL,"<float>","please input flaot");
    mqtt_args.end=arg_end(1);

    esp_console_cmd_t cmd={
        .func=&hello_world_cmd,
        .command="hello",
        .help="Printf a hello world",
        .hint=NULL
    };
    esp_console_cmd_register(&cmd);

    esp_console_cmd_t cmd1={
        .func=&hello_world_arg_cmd,
        .command="hello_args",
        .help="Printf a hello world with args int",
        .hint=NULL
    };
    esp_console_cmd_register(&cmd1);

    esp_console_cmd_t cmd2={
        .func=&get_heap_cmd,
        .command="heap",
        .help="Get Heap",
        .hint=NULL
    };
    esp_console_cmd_register(&cmd2);

    esp_console_cmd_t cmd3={
        .func=&led_change_cmd,
        .command="led_change",
        .help="change led2 gpio level",
        .hint=NULL
    };
    esp_console_cmd_register(&cmd3);

    esp_console_cmd_t cmd6={
        .func=&get_task_num_state,
        .command="get_task",
        .help="printf task num and state",
        .hint=NULL
    };
    esp_console_cmd_register(&cmd6);

    esp_console_cmd_t cmd7={
        .func=&supsend_task_cmd,
        .command="suspend_task",
        .help="suspend task",
        .hint=NULL
    };
    esp_console_cmd_register(&cmd7);

    esp_console_cmd_t cmd8={
        .func=&resume_task_cmd,
        .command="resume_task",
        .help="resume task",
        .hint=NULL
    };
    esp_console_cmd_register(&cmd8);

    esp_console_cmd_t cmd9={
        .func=&mqtt_cmd,
        .command="mqtt_send",
        .help="mqtt to send float",
        .hint=NULL
    };
    esp_console_cmd_register(&cmd9);

    esp_console_cmd_t cmd10={
        .func=&start_mqtt_cmd,
        .command="mqtt_start",
        .help="start mqtt",
        .hint=NULL
    };
    esp_console_cmd_register(&cmd10);


    esp_console_cmd_t cmd11={
        .func=&random_cmd,
        .command="random",
        .help="random a number",
        .hint=NULL
    };
    esp_console_cmd_register(&cmd11);



    //start repl
    esp_console_start_repl(repl);
}