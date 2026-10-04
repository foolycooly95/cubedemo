typedef struct {
    double x;
    double y;
} vec2;

typedef struct {
    double x;
    double y;
    double z;
} vec3;

typedef enum {
 SCENE_STATIC,
 SCENE_CUBE,
} Scene;

typedef enum {
    CUBE_ROTATE_RIGHT,
    CUBE_ROTATE_DOWN,
    CUBE_ROTATE_LEFT,
    CUBE_ROTATE_UP,
} CubeRotation;
