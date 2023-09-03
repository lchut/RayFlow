# RayFlow
A path tracer.

# Feature
- SAH-based BVH
- Disney BSDF
- Multiple importance sampling
- Path tracing
- Bidirectional path tracing

# Gallery
bedroom
![](result/bedroom-bdpt-256.png)

cornell-box
|  PT-1024 spp   | BDPT-256 spp  |
|  ----  | ----  |
| ![](result/cornellbox-pt-1024.png)  | ![](result/cornellbox-bdpt-256.png) |

cornell-box-dielectric

|  PT-256 spp   | BDPT-256 spp  |
|  ----  | ----  |
| ![](result/cornell-box-easy-pt-256.png)  | ![](result/cornell-box-easy-bdpt-256.png) |

|  Diffuse   | Mirror  |
|  ----  | ----  |
|![](result/cornell-box-diffuse-bdpt-64.png)|![](result/cornell-box-mirror-bdpt-256.png)|
|Glass|
|![](result/cornell-box-dielectric-bdpt-1024.png)|

Disney Material
|  Disney diffuse   | Disney glass  |
|  ----  | ----  |
| ![](result/cornell-box-monster-disneydiffuse-bdpt-256.png)  | ![](result/cornell-box-monster-disneyglass-bdpt-1024.png) |
|  Disney metal   | Disney clearcoat  |
| ![](result/cornell-box-monster-disneymetal-bdpt-256.png)  | ![](result/cornell-box-monster-disneyclearcoat-bdpt-64.png) |
|  Disney sheen   |
| ![](result/cornell-box-monster-disneysheen-bdpt256.png)  |

Rough Material
|  Rough conductor  | Rough deielectric  | 
|  ----  | ----  |
|![](result/cornell-box-monster-roughconductor-bdpt-256.png)| ![](result/cornell-box-monster-roughdielectric-bdpt-1024.png)|
|Rough plastic |
|![](result/cornell-box-monster-roughplastic-bdpt-1024.png)|