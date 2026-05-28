# 放在 D:\AutoRubbishRobot\Orlando\main\CMakeLists.txt
# 这是告诉编译器，main 文件夹里有哪些代码，以及它依赖了哪些组件

idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES "actuator" "freertos" "esp_timer"
)