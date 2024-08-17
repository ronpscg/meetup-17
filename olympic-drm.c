/**
 * This is a trivial example program that uses DRM to draw to a display, without any dependencies other than libdrm.
 * The printouts are meant to escort a learning sessions, when you use QEMU with a serial console, or when you ssh into a machine, etc.
 * Otherwise, unless there is an error, you can ignore it.
 *
 * To exit the program, when it is done, hit any key in the tty where you ran the program.
 * If you ran it on a display related tty (e.g. one of the VTs), chvt to another VT,and then change back, to see what you expect (e.g. your shell, e.g. in a Dekstop distro).
 *
 * Written by Ron Munitz for the PSCG Linux/Android graphics courses.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <math.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/*
 * Hardly relying on the values in a snippet of drm_mode.h taken in my computer, August 4th 2024.
 * We know that there are no holes, and the defines corresponding to the names start at 0, so that is how we number them.
 * If that changes, it will of course be an error, but this code is made for a nice demo in a talk given during the 2024 Olympic games period
 */
#define MAX_DRM_MODE_CONNECTOR (DRM_MODE_CONNECTOR_USB+1)	
static char *DRM_MODE_CONNECTOR_NAMES[MAX_DRM_MODE_CONNECTOR]={
	"Unknown", "VGA", "DVII", "DVID", "DVIA", "Composite", "SVIDEO", "LVDS", "Component", "9PinDIN", 
	"DisplayPort", "HDMIA", "HDMIB", "TV", "eDP",
	"VIRTUAL",
	"DSI", "DPI", "WRITEBACK", "SPI", "USB"
};


/*
 * Similar comment, snippet of /usr/include/xf86drmMode.h
 * Here the numbers are shorter, and the enums start from 1
 */
static char *DRM_MODE_CONNECTION[]={"N/A","CONNECTED", "DISCONNECTED", "UNKNOWNCONNECTION" };



struct buf_info {
    uint32_t *map;
    uint32_t width;
    uint32_t height;
    uint32_t pitch; /* pitch/stride is essentially <number of pixels in a row> * depth */
    uint32_t size;
};

static int modeset_create_fb(int fd, drmModeModeInfo *mode, uint32_t *fb_id, struct buf_info *buf)
{
    struct drm_mode_create_dumb create = {};
    struct drm_mode_map_dumb map = {};
    uint32_t handle;
    int ret;

    create.width = mode->hdisplay;
    create.height = mode->vdisplay;
    create.bpp = 32;
    ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &create);
    if (ret < 0) {
        perror("Failed to create dumb buffer");
        return -1;
    }

    handle = create.handle;
    buf->pitch = create.pitch;
    buf->size = create.size;
    buf->width = create.width;
    buf->height = create.height;

    ret = drmModeAddFB(fd, create.width, create.height, 24, 32, create.pitch,
               handle, fb_id);
    if (ret) {
        perror("Failed to add FB");
        return -1;
    }

    map.handle = handle;
    ret = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map);
    if (ret) {
        perror("Failed to map dumb buffer");
        return -1;
    }

    buf->map = mmap(0, create.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, map.offset);
    if (buf->map == MAP_FAILED) {
        perror("Failed to mmap buffer");
        return -1;
    }

    return 0;
}


void draw_circle(struct buf_info *buf, int x0, int y0, int radius, uint32_t color, uint32_t background)
{
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            int dist = x*x + y*y;
            if (dist <= radius*radius && dist >= (radius-5)*(radius-5)) {
                int draw_x = x0 + x;
                int draw_y = y0 + y;
                if (draw_x >= 0 && draw_x < buf->width && draw_y >= 0 && draw_y < buf->height) {
                    buf->map[draw_y * buf->width + draw_x] = color;
                }
            }
        }
    }
}

void draw_olympic_rings(struct buf_info *buf)
{
    int radius = buf->height / 6;
    int y_top = buf->height / 2 - radius / 2;
    int y_bottom = buf->height / 2 + radius / 2;
    int x_start = (buf->width - (5 * radius)) / 2;

    uint32_t colors[] = {
        0xFF0000FF,  // Blue
        0xFF000000,  // Black
        0xFFFF0000,  // Red
        0xFFFFFF00,  // Yellow
        0xFF00FF00   // Green
    };
    uint32_t background = 0xFFFFFFFF;  // White background

    int x_positions[] = {0, 2, 4, 1, 3};
    int y_positions[] = {0, 0, 0, 1, 1};

    // Draw background circles first
    for (int i = 0; i < 5; i++) {
        int x = x_start + x_positions[i] * radius;
        int y = y_positions[i] ? y_bottom : y_top;
        draw_circle(buf, x, y, radius, background, background);
    }

    // Draw colored rings
    for (int i = 0; i < 5; i++) {
        int x = x_start + x_positions[i] * radius;
        int y = y_positions[i] ? y_bottom : y_top;
        draw_circle(buf, x, y, radius, colors[i], background);
    }
}


void optional_demonstration(int fd, drmModeModeInfo *mode, uint32_t *fb_id, struct buf_info *buf, uint32_t crtc_id, drmModeConnector *connector); // forward declaration for a class example

int main(int argc, char **argv) {
    int fd = open("/dev/dri/card0", O_RDWR);
    if (fd < 0) {
        perror("Failed to open DRM device");
        return 1;
    }

    drmModeRes *resources = drmModeGetResources(fd);
    if (!resources) {
        perror("Failed to get DRM resources");
        close(fd);
        return 1;
    }

    drmModeConnector *connector = NULL;
    for (int i = 0; i < resources->count_connectors; i++) {
        connector = drmModeGetConnector(fd, resources->connectors[i]);
	printf("Can use xf86 functions:: %s\n", drmModeGetConnectorTypeName(connector->connector_type));
	printf("connection=%s, connector_type=%s, connector_type_id=%d\n", 
			DRM_MODE_CONNECTION[connector->connection],
		       	DRM_MODE_CONNECTOR_NAMES[connector->connector_type], 
			connector->connector_type_id);

#ifdef QEMUONLY
        if (connector->connection == DRM_MODE_CONNECTED &&
            connector->connector_type == DRM_MODE_CONNECTOR_VIRTUAL &&
            connector->connector_type_id == 1) {
		break
        }
#else 
	if (connector->connection == DRM_MODE_CONNECTED) {
		// go for the first one - unless it is an internal display (comment out the if block and just break otherwise...)
		// I am not sure what will be the setup in classs, but this should make them see it on the display of a VT.
		// do it just once at some point...
		if (connector->connector_type != DRM_MODE_CONNECTOR_eDP) {
			break;
		}
	}
#endif /*QEMUONLY*/	
        drmModeFreeConnector(connector);
        connector = NULL;
    }

    if (!connector) {
        fprintf(stderr, "Failed to find the requested connector\n"); // in QEMU - Virtual-1
        drmModeFreeResources(resources);
        close(fd);
        return 1;
    }

    uint32_t crtc_id = resources->crtcs[0];  // Use the first CRTC

    if (connector->count_modes > 0) {
        drmModeModeInfo mode = connector->modes[0];  // Use the first available mode
        printf("Setting mode: %dx%d\n", mode.hdisplay, mode.vdisplay);
        
        uint32_t fb_id;
        struct buf_info buf = {};
        if (modeset_create_fb(fd, &mode, &fb_id, &buf) < 0) {
            fprintf(stderr, "Failed to create framebuffer\n");
            drmModeFreeConnector(connector);
            drmModeFreeResources(resources);
            close(fd);
            return 1;
        }

        // Clear the framebuffer to white
        memset(buf.map, 0xFF, buf.size);

        // Draw Olympic rings
        draw_olympic_rings(&buf);

        if (drmModeSetCrtc(fd, crtc_id, fb_id, 0, 0, &connector->connector_id, 1, &mode) < 0) {
            perror("Failed to set CRTC");
        } else {
            printf("Mode set successfully: %dx%d\n", mode.hdisplay, mode.vdisplay);
        }

	// DEMONSTRATION OF SOME CONCEPTS - NOT FOR A FIRST DEMO. Uncomment to avoid that
	optional_demonstration(fd, &mode, &fb_id, &buf, crtc_id, connector); 

        // Keep the program running to display the image
	printf("Press any key to continue on the tty you ran %s at", argv[0]);
        getchar();

        munmap(buf.map, buf.size);
    } else {
        fprintf(stderr, "No modes available\n");
    }

    drmModeFreeConnector(connector);
    drmModeFreeResources(resources);
    close(fd);

    return 0;
}


/**
 * This example shows potential effects when you want to draw several frames.
 * If you understand the problem, and the fix, you will have the required motivation for using double buffering (or more),
 * and it is beyond of the scope of this demonstration, that aims to "teach the DRM API in less than a minute"
n* 
 * So NAIVE_PAGE_FLIP is essentially "any" page flip. The recommendation is to run the application once with NAIVE_PAGE_FLIP defined, and once
 * without, but it is easy to forget, so just change the define/undef below and rebuild.
 */
void optional_demonstration(int fd, drmModeModeInfo *mode, uint32_t *fb_id, struct buf_info *buf, uint32_t crtc_id, drmModeConnector *connector) {
	#define NAIVE_PAGE_FLIP
	sleep(3); // give opportunity to see the rings
	for (int i=0; i<20; i++) {
        	memset(buf->map, i*10, buf->size);
#ifdef NAIVE_PAGE_FLIP
		if (drmModeSetCrtc(fd, crtc_id, *fb_id, 0, 0, &connector->connector_id, 1, mode) < 0) {
			perror("Failed to set CRTC");
		} else {
			printf("Mode set successfully: %dx%d\n", mode->hdisplay, mode->vdisplay);
		}
#endif
		usleep(100000);
	}

	printf("Drawing olympic rings again\n");
	draw_olympic_rings(buf);

#ifdef NAIVE_PAGE_FLIP
		if (drmModeSetCrtc(fd, crtc_id, *fb_id, 0, 0, &connector->connector_id, 1, mode) < 0) {
			perror("Failed to set CRTC");
		} else {
			printf("Mode set successfully: %dx%d\n", mode->hdisplay, mode->vdisplay);
		}
#endif
}
