# Apache Video Thumb Extractor Module

The Apache Video Thumb Extractor Module is designed to extract an image from a video at a specific second, allowing for resizing, cropping, and extracting a storyboard (multiple video frames on a sprite) from the video.

### Extract a Storyboard Frames (with or without pagination):

```bash
https://your_host/thumbnails/bigbunny.ogg?height=100&split=30
https://your_host/thumbnails/bigbunny.ogg?height=100&split=30&columns=5
https://your_host/thumbnails/bigbunny.ogg?height=100&split=30&columns=5&currentPage=1&pageSize=10
```

![Sample Storyboard](http://github.com/dayvson/Apache-Get-Video-Thumbnail/blob/master/sample_storyboard.jpg?raw=true)

### Extract a single video frame:

```bash
https://your_host/thumbnails/bigbunny.ogg?second=30
https://your_host/thumbnails/bigbunny.ogg?second=30&width=640
https://your_host/thumbnails/bigbunny.ogg?second=30&width=640&height=360
```

![Sample Frame](http://github.com/dayvson/Apache-Get-Video-Thumbnail/blob/master/sample_frame.jpg?raw=true)


## Requirements

This module depends on certain libraries (headers and shared objects) that need to be installed beforehand. To install FFmpeg from source with specific versions, follow these steps:

```bash
git clone git://git.videolan.org/ffmpeg
cd ffmpeg
git checkout n0.8.7 -b tag_n0.8.7
./configure --prefix=/usr --disable-ffserver --disable-ffplay --enable-shared
make
make install
```

## Dependencies

The following libraries are commonly distributed with [FFmpeg](http://ffmpeg.org) or in [LIBAV](http://libav.org):
- avformat >= 53.4.0
- avcodec >= 53.7.0
- avutil >= 51.9.1
- swscale >= 2.0.0
- libjpeg - [libjpeg](http://libjpeg.sourceforge.net)

## How to Compile

You can run tests and see the saved files video.jpg and storyboard.jpg from the demo video:

```bash
make test
```

To compile and install this module on your Apache server, run the following command. This action will create a `video_thumbnail.so` file and place it in your Apache module folder. Please check the `defines.mk` file to set your installation path. 

```bash
sudo make install
```

## Configuration

After installation, to enable this module, edit the `httpd.conf` file and add the following line in the module section:

```apache
LoadModule videothumb_module [module_path]/mod_videothumb.so
```

- `VideoThumb_Enabled true`: Enable the videothumb module
- `VideoThumb_MediasPath /www/my_videos/`: Path for videos in your filesystem
- `VideoThumb_AppPath /thumbnails/`: Path for which you want the module to respond (the URI path)
- `VideoThumb_JpegQuality 90`: JPEG compression quality

## How to Use

After enabling this module, you can access it using the following URLs. This module can receive several parameters:
- `second`: Second of the video (this module gets the nearest keyframe to the second) (Optional parameter in storyboard)
- `width`: Resize width of the video frame (optional)
- `height`: Resize height of the video frame (optional)
- `split`: Create a storyboard with a specified number of frames
- `columns`: Define the number of columns in the storyboard
- `currentPage`: Define the current page of the storyboard
- `pageSize`: Define the number of frames one storyboard page must have

If you don't use width or height parameters, the frame dimensions will be equal to the video. If you use the split parameter, you can't use the second parameter, as this function returns a sprite with frames in a JPEG file.
