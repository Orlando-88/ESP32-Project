# 放在 D:\AutoRubbishRobot\Orlando\components\actuator\CMakeLists.txt

idf_component_register(
    SRCS "actuator_driver.c"
    INCLUDE_DIRS "include"
    REQUIRES freertos driver esp_timer
)