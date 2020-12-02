#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <ospray/ospray.h>

// helper function to write the rendered image as PPM file
void writePPM(const char *fileName,
              const int xsize,
              const int ysize,
              const uint32_t *pixel)
{
    fprintf(stderr, "Writing to %s\n", fileName);

    FILE *file = fopen(fileName, "wb");
    if (!file) {
        fprintf(stderr, "fopen('%s', 'wb') failed: %d", fileName, errno);
        return;
    }
    fprintf(file, "P6\n%i %i\n255\n", xsize, ysize);
    unsigned char *out = (unsigned char *)alloca(3*xsize);
    for (int y = 0; y < ysize; y++) {
        const unsigned char *in = (const unsigned char *)&pixel[(ysize-1-y)*xsize];
        for (int x = 0; x < xsize; x++) {
            out[3*x + 0] = in[4*x + 0];
            out[3*x + 1] = in[4*x + 1];
            out[3*x + 2] = in[4*x + 2];
        }
        fwrite(out, 3*xsize, sizeof(char), file);
    }
    fprintf(file, "\n");
    fclose(file);
}

int
main(int argc, const char **argv) {
    OSPError err;
    OSPGeometry geometry;
    OSPData position;
    OSPGeometricModel model;
    OSPMaterial material;
    OSPGroup group;
    OSPInstance instance;
    OSPWorld world;
    OSPRenderer renderer;
    OSPLight light;
    OSPCamera camera;
    OSPFrameBuffer frameBuffer;
    OSPFuture future;
    uint32_t *pixels;

    err = ospInit(&argc, argv);
    if (err != OSP_NO_ERROR) {
        fprintf(stderr, "Error!\n");
        return 1;
    }

    geometry = ospNewGeometry("sphere");
    ospSetParam(geometry, "radius", OSP_FLOAT, &(float){ 0.1 });
    position = ospNewSharedData(
        (float[]){
            0.0, 0.0, 0.0,
            1.0, 0.0, 0.0,
            0.0, 1.0, 0.0,
            0.0, 0.0, 1.0,
        },
        OSP_VEC3F, 4, 0, 1, 0, 1, 0
    );
    ospCommit(position);
    ospSetParam(geometry, "sphere.position", OSP_DATA, (OSPData[]){ position });
    ospRelease(position);
    ospCommit(geometry);

    material = ospNewMaterial("pathtracer", "obj");
    ospCommit(material);

    model = ospNewGeometricModel(geometry);
    ospSetParam(model, "material", OSP_MATERIAL, (OSPMaterial[]){ material });
    ospCommit(model);

    group = ospNewGroup();
    ospSetParam(group, "geometry", OSP_DATA, &(OSPData){ ospNewSharedData(
        (OSPObject[]){
            model,
        }, OSP_OBJECT, 1, 0, 1, 0, 1, 0) });
    ospCommit(group);

    instance = ospNewInstance(group);
    ospCommit(instance);

    light = ospNewLight("sphere");
    ospSetParam(light, "position", OSP_VEC3F, (float[]){ 0.5, 0.5, 0.5 });
    ospSetParam(light, "radius", OSP_FLOAT, (float[]){ 0.5 });
    ospCommit(light);

    world = ospNewWorld();
    ospSetParam(world, "instance", OSP_DATA, &(OSPData){ ospNewSharedData(
        (OSPObject[]){
            instance,
        }, OSP_OBJECT, 1, 0, 1, 0, 1, 0) });
    ospSetParam(world, "light", OSP_DATA, &(OSPData){ ospNewSharedData(
        (OSPObject[]){
            light,
        }, OSP_OBJECT, 1, 0, 1, 0, 1, 0) });
    ospCommit(world);

    renderer = ospNewRenderer("pathtracer");
    ospSetParam(renderer, "backgroundColor", OSP_VEC3F, (float[]){ 0.0, 1.0, 0.0 });
    ospCommit(renderer);

    camera = ospNewCamera("perspective");
    ospSetParam(camera, "position", OSP_VEC3F, &(float[]){ -1.0, -1.0, -1.0 });
    ospSetParam(camera, "direction", OSP_VEC3F, &(float[]){ +1.0, +1.0, +1.0 });
    ospSetParam(camera, "up", OSP_VEC3F, &(float[]){ 0.0, 1.0, 0.0 });
    ospSetParam(camera, "nearClip", OSP_FLOAT, &(float){ 1.0 });
    //ospSetParam(camera, "imageStart", OSP_VEC2F, &(float[]){ 0.0, 0.0 });
    //ospSetParam(camera, "imageEnd", OSP_VEC2F, &(float[]){ 1.0, 1.0 });
    ospCommit(camera);

    frameBuffer = ospNewFrameBuffer(256, 256, OSP_FB_SRGBA, OSP_FB_COLOR);
    ospCommit(frameBuffer);

    future = ospRenderFrame(frameBuffer, renderer, camera, world);
    ospWait(future, OSP_TASK_FINISHED);

    pixels = (uint32_t *)ospMapFrameBuffer(frameBuffer, OSP_FB_COLOR);

    char name[1024];
    snprintf(name, sizeof(name), "%s.ppm", argv[0]);
    writePPM(name, 256, 256, pixels);

    ospUnmapFrameBuffer(pixels, frameBuffer);
    
    ospShutdown();
    
    return 0;
}

// vim: sta:et:sw=4:ts=4:sts=4:ai
