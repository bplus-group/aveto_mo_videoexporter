<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="https://www.b-plus.com/de/home">
    <img src="https://www.b-plus.com/fileadmin/data_storage/images/b-plus_Logo.png" alt="Logo" width="150" height="150">
  </a>
  
  <h3 align="center">AVETO.app VideoExporter Measurement Object</h3>

  <p align="center">
    Measurement object for AVETO.app Visualization to export video streams to .avi files
    <br />
    <a href="#usage">View Usage</a>
    ·
    <a href="https://github.com/bplus-group/aveto_mo_videoexporter/issues">Report Bug</a>
    ·
    <a href="https://github.com/bplus-group/aveto_mo_videoexporter/issues">Request Feature</a>
  </p>
</div>
<br />

<!-- PROJECT SHIELDS -->
<div align="center">

  [![LinkedIn][linkedin-shield]][linkedin-url]
  [![Stars][star-shield]][star-url]

</div>

<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li>
  </ol>
</details>

---

## About The Project

**Object:** Exporter Object  
**Name:** Video Exporter  
**Input:** Native Image Connector named „IMAGE_RGBa8“  
**Output:**  

| <span style="font-weight:normal">Container</span> | <span style="font-weight:normal">AVI</span> |
| ------------------------------------------------- | ------------------------------------------- |
| Codec                                             | MJPG                                        |


**Properties:**

| Property Name            | Type        | Default  | Description                                                                                                    |
| ------------------------ | ----------- | -------- | -------------------------------------------------------------------------------------------------------------- |
| Allow Encoding           | bool        | true     | **true:** allow encoding <br />**false:** stop encoding                                                        |
| Frames per second        | uint32_t    | 25       | FPS of the exporting video;<br /> every incoming packet is one frame                                           |
| Output Filename          | std::string | „Output“ | Name of the exporting video                                                                                    |
| Output Directory         | std::string | „C:\“    | Loaction to the output directory                                                                               |
| Use Numeration as suffix | bool        | true     | **true:** Add counter to the end of the filename <br /> **false:** Use only „Output Filename“ property as name |
| Timeout[ms]              | uint32_t    | 2000     | Maximum number of milliseconds between incoming packages. If the delay is greater the export ends.             |


By default after creation when data packets first come in over a connection they will be exported. After the export of a video is finished the property `Allow Encoding` has to be manually set to *true* to start another video export or a new connection has to be established.

Video export ends automatically if the delay between two packets is greater than the value of proptery `Timeout[ms]` or the connection to the input connector is severed.

When property `Use Numeration` as suffix is *true* the exported video on default settings is `C:\Output001.avi`. If it is *false* the video is `C:\Output.avi`.

> **Note**
> If no file can be generated e.g. due to missing permissions at the target location there will be no warning. The only indication is that the counter gets incremented for every incoming package that could not be written and then the first working export after will start at a higher number than 001.

<p align="right"><a href="#top">Back to top</a></p>

### Built With

**3rd party components:**  
OpenCV 4.5.3  
**Used binary:**  
opencv_world453.dll  

<p align="right"><a href="#top">Back to top</a></p>

## Getting Started

Clone this repository and make sure you fulfill following requirements

### Prerequisites

- Installed AVETO.app >= 2.4.0
- Set AVETO_SDK_PATH as environment variable which by default points to the folder `C:\Program Files\b-plus\AVETO.app\sdk`
- Installed Windows SDK-Version 10.0.17763.0
- Installed OpenCV 4.5.3

<p align="right"><a href="#top">Back to top</a></p>

### Installation


1. Install AVETO.app
2. Clone the repo
   ```sh
   git clone https://github.com/bplus-group/aveto_mo_videoexporter.git
   ```
3. Install OpenCV 4.5.3. and set the environment variable to the installation location
4. Adapt the include and library directories in the Visual Studio Solution that it finds the OpenCV dependencies 
5. Build the project
6. Copy VideoExporter.dll to your AVETO MO directory (default: `%USERPROFILE%\Documents\AVETO\measobj\`)

<p align="right"><a href="#top">Back to top</a></p>

## Usage

Right-click in the node graph editor of the configuration manager and choose *Exporter Objects -> Video Exporter* as seen below.

![videoexporter][videoexporter]



By clicking on the Video Exporter MO the *output directory* and *output filename* can be edited in the property list.

  
> **Note**  
> Make sure to have write permissions to the output directory. C:\ may not work!
  

![graph_videoexporter][graph_videoexporter]

After starting a recording (with a playback) or by connecting to a live source the video source will be converted as an .avi-file to the desired location.

<p align="right"><a href="#top">Back to top</a></p>

## Contributing


If you have a suggestion that would improve this, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".  
Don't forget to give the project a star! Thanks again!


1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/NewFeature`)
3. Commit your Changes (`git commit -m 'Add some NewFeature'`)
4. Push to the Branch (`git push origin feature/NewFeature`)
5. Open a Pull Request

<p align="right"><a href="#top">Back to top</a></p>

## License

Check License information. See `LICENSE` for more information.

<p align="right"><a href="#top">Back to top</a></p>








<!---Links And Images -->
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&color=808080
[linkedin-url]: https://de.linkedin.com/company/b-plus-group
[star-shield]: https://img.shields.io/github/stars/bplus-group/aveto_mo_videoexporter.svg?style=for-the-badge&color=144E73&labelColor=808080
[star-url]: https://github.com/bplus-group/aveto_mo_videoexporter
[videoexporter]: ./docs/images/videoexporter.png "VideoExporter"
[graph_videoexporter]: ./docs/images/graph_editor_videoexporter.png "Logo Title Text 2"
