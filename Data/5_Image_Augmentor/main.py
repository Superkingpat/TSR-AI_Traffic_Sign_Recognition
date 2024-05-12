import numpy as np
import imgaug.augmenters as iaa
import cv2

aug_pipeline = iaa.Sequential([
        ###   COLOR
        iaa.MultiplySaturation((0.8, 1.5)),
        iaa.MultiplyBrightness((0.7, 1.5)),
        iaa.AddToHue((-5, 5)),
        #iaa.ContrastNormalization((0.5, 2.0)),
        #iaa.ChangeColorTemperature((4000,10000)), ### doesn't work :(

        ###   Obstructions
        iaa.Sometimes(0.3, iaa.Cutout(fill_mode="constant", cval=(0, 255), fill_per_channel=(0.3), nb_iterations=(1, 10), size=(0.1, 0.3), squared=False)),

        ###   BLUR
        iaa.OneOf([
            iaa.GaussianBlur(sigma=(0, 4.0)),  
            iaa.MotionBlur(k=15, angle=[-90, 90]),
            iaa.Noop(),
        ]),

        ###   TRANSFORMATIONS
        iaa.Affine(scale=0.8),
        iaa.Sometimes(0.8, iaa.PerspectiveTransform(scale=(0.05, 0.1), keep_size=False)),
        iaa.Sometimes(0.5, iaa.ShearY((-15, 15))),
        iaa.Sometimes(0.5, iaa.CropAndPad(percent=(-0.1, 0.1))),
        iaa.Sometimes(0.5, iaa.Affine(rotate=(-20, 20))),
        iaa.Sometimes(0.5, iaa.Affine(shear=(-10, 10))),
        iaa.Sometimes(0.5, iaa.Affine(scale={"x": (1, 1.5), "y": (1, 1.5)}),),
        

        ### SEGMENTATIONS
        iaa.OneOf([
            iaa.RegularGridVoronoi((10, 30), 20, p_drop_points=0.0, p_replace=0.3, max_size=None),
            iaa.AveragePooling(5),
            iaa.UniformVoronoi(250, p_replace=0.3, max_size=None),
            iaa.Noop(),
        ]),


        ### Simulate Weather
        iaa.OneOf([
            iaa.Sequential([
                iaa.Clouds(),
            ]),
            iaa.Sequential([
                iaa.Snowflakes(flake_size=(0.1, 0.4), speed=(0.01, 0.05)),
                iaa.Sometimes(0.2, iaa.ElasticTransformation(alpha=(0, 4.0), sigma=0.1)),  ### ICE EFFECT
            ]),
            iaa.Sequential([
                iaa.Fog(),
            ]),
            iaa.Sequential([
                iaa.Rain(speed=(0.1, 0.3)),
                iaa.Sometimes(0.8, iaa.Clouds()),
            ]),
            iaa.Noop(),
        ]),
    ])

def augment_images(image_path, num_of_images, show_output = False):
    image = cv2.imread(image_path)  

    if len(image.shape) == 2:
        image = cv2.cvtColor(image, cv2.COLOR_GRAY2RGB)
    elif len(image.shape) == 3 and image.shape[2] == 1:
        image = cv2.cvtColor(image, cv2.COLOR_GRAY2RGB)

    augmented_images = aug_pipeline(images=[image] * num_of_images)

    if show_output:
        # Calculate number of rows and columns for the grid
        num_images = len(augmented_images)
        aspect_ratio = augmented_images[0].shape[1] / augmented_images[0].shape[0]
        num_cols = int(np.sqrt(num_images * aspect_ratio))
        num_rows = int(np.floor(num_images / num_cols))

        # Resize images to fit 2K resolution
        resize_width = 2000 // num_cols
        resize_height = 1000 // num_rows
        resized_images = [cv2.resize(img, (resize_width, resize_height)) for img in augmented_images]

        # Stack images into a grid
        stacked_rows = [np.hstack(resized_images[i * num_cols:(i + 1) * num_cols]) for i in range(num_rows)]
        grid_image = np.vstack(stacked_rows)
        cv2.imshow('Generated', grid_image)
        cv2.waitKey(0)
        cv2.destroyAllWindows()

    return augmented_images

augment_images(r"C:\Users\patri\OneDrive\Dokumenti\GITHUB\TSR-AI_Traffic_Sign_Recognition\Data\5_Image_Augmentor\Signs\avtobusno-postajalisce-2433-144.png", 100, True)