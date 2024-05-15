import numpy as np
import imgaug.augmenters as iaa
import cv2
import random
from line_detection import load_image, get_random_position
from numba import njit
from os.path import join
from labeler import create_dataset
cColor = 3

aug_pipeline_sign = iaa.Sequential([
        ###   Obstructions
        iaa.Sometimes(0.3, iaa.Cutout(fill_mode="constant", cval=(0, 255), fill_per_channel=(0.3), nb_iterations=(1, 10), size=(0.1, 0.3), squared=False)),

        ###   TRANSFORMATIONS
        iaa.Affine(scale=0.5, cval=cColor, mode="constant"),
        iaa.Sometimes(0.8, iaa.PerspectiveTransform(scale=(0.05, 0.1), keep_size=False, cval=cColor, mode="constant")),
        iaa.Sometimes(0.5, iaa.ShearY((-15, 15), cval=cColor, mode="constant")),
        iaa.Sometimes(0.5, iaa.CropAndPad(percent=(-0.1, 0.1), pad_cval=cColor, pad_mode="constant")),
        iaa.Sometimes(0.5, iaa.Affine(rotate=(-20, 20), cval=cColor, mode="constant")),
        iaa.Sometimes(0.5, iaa.Affine(shear=(-10, 10), cval=cColor, mode="constant")),
        iaa.Sometimes(0.5, iaa.Affine(scale={"x": (1, 1.5), "y": (1, 1.5)}, cval=cColor, mode="constant")),
    ])

aug_pipeline_sign_c = iaa.Sequential([
        ###   COLOR
        iaa.MultiplySaturation((0.8, 1.5)),
        iaa.MultiplyBrightness((0.7, 1.5)),
        iaa.AddToHue((-5, 5)),
        #iaa.ContrastNormalization((0.5, 2.0)),
        #iaa.ChangeColorTemperature((4000,10000)), ### doesn't work :(
    ])

aug_pipeline_img = iaa.Sequential([
        ###   BLUR
        iaa.OneOf([
            iaa.GaussianBlur(sigma=(0, 4.0)),  
            iaa.MotionBlur(k=15, angle=[-90, 90]),
            iaa.Noop(),
        ]),

        ### SEGMENTATIONS
        iaa.OneOf([
            iaa.AveragePooling((1, 7)),
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
            iaa.Noop(),
        ]),
    ])

@njit
def crop_image(image, center, size):
    center_x, center_y = center
    
    half_width = size[0] // 2
    half_height = size[1] // 2

    if center_x - half_width < 0:
        center_x = half_width
    elif center_x + half_width > image.shape[1]:
        center_x = image.shape[1] - half_width
        
    if center_y - half_height < 0:
        center_y = half_height
    elif center_y + half_height > image.shape[0]:
        center_y = image.shape[0] - half_height
    
    left = center_x - half_width
    upper = center_y - half_height
    right = center_x + half_width
    lower = center_y + half_height
    
    cropped_image = image[upper:lower, left:right]
    
    return cropped_image

@njit
def add_background(overlay_image, bg_image, center, crop = True):
    if crop:
        n = random.randint(144, 244)
        size = (n, n)
        bg_image = crop_image(bg_image, center, size)

    overlay_height, overlay_width, _ = overlay_image.shape

    x_offset = center[0] - 122
    y_offset = center[1] - 122

    for y in range(overlay_height):
        for x in range(overlay_width):
            alpha = overlay_image[y, x, 3] / 255.0
            bg_image[y + y_offset, x + x_offset] = alpha * overlay_image[y, x, 0:3] + (1 - alpha) * bg_image[y + y_offset, x + x_offset]
    return bg_image

def augment_images(image_path, num_of_images, env_list, env_paths, show_output = False):
    env_path, env_path_m = env_paths
    overlay = cv2.imread(image_path, cv2.IMREAD_UNCHANGED)
    overlay_rgb = overlay[:, :, :3]
    mask = overlay[:, :, 3]
    mask = (mask == 0)
    overlay_rgb[mask] = [cColor, cColor, cColor]

    del mask, overlay

    augmented_images = aug_pipeline_sign(images=[overlay_rgb] * num_of_images) #pipeline

    augmented_masks = [np.all(image == (cColor, cColor, cColor), axis=-1) for image in augmented_images]

    augmented_images = aug_pipeline_sign_c(images=augmented_images) #pipeline

    for i in range(len(augmented_images)):
        env = random.choice(env_list)
        load_image(join(env_path_m, env))
        bg_img = cv2.imread(join(env_path, env))    
        center = get_random_position(False, 10)
        alpha_channel = np.ones((augmented_images[i].shape[0], augmented_images[i].shape[1], 1), dtype=np.uint8) * 255
        augmented_images[i] = np.concatenate((augmented_images[i], alpha_channel), axis=-1)
        augmented_images[i][augmented_masks[i]] = [0, 0, 0, 0]
        augmented_images[i] = add_background(augmented_images[i], bg_img, center)
    
    augmented_images = aug_pipeline_img(images=augmented_images) #pipeline

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

def augment_images_arg(args):
    image_path, num_of_images, env_list, env_paths, show_output = args
    return augment_images(image_path, num_of_images, env_list, env_paths, show_output)

def augment_images_complete(signs_list, num_of_signs, env, env_paths, show_output = False):
    env_path, env_path_m  = env_paths
    load_image(join(env_path_m, env))
    bg_img = cv2.imread(join(env_path, env))
    centers = []
    sizes = []

    for _ in range(num_of_signs):
        overlay = cv2.imread(random.choice(signs_list), cv2.IMREAD_UNCHANGED)
        overlay_rgb = overlay[:, :, :3]
        mask = overlay[:, :, 3]
        mask = (mask == 0)
        overlay_rgb[mask] = [cColor, cColor, cColor]

        augmented_image = aug_pipeline_sign(image=overlay_rgb) #pipeline

        augmented_mask = np.all(augmented_image == (cColor, cColor, cColor), axis=-1)

        augmented_image = aug_pipeline_sign_c(image=augmented_image) #pipeline
        
        center = get_random_position(True, 20)
        if len(center) == 0: break
        centers.append(np.array([center[0], center[1]]))
        sizes.append(np.array([augmented_image.shape[0], augmented_image.shape[1]]))
        
        alpha_channel = np.ones((augmented_image.shape[0], augmented_image.shape[1], 1), dtype=np.uint8) * 255
        augmented_image = np.concatenate((augmented_image, alpha_channel), axis=-1)
        augmented_image[augmented_mask] = [0, 0, 0, 0]
        bg_img = add_background(augmented_image, bg_img, center, False)
    
    bg_img = aug_pipeline_img(image=bg_img) #pipeline
    #cv2.imwrite(join(PATH_TO_GEN_FOLDER,f"{env.split(".")[0]}_gen.jpg"), bg_img)
    create_dataset([bg_img], [np.array(centers)], [np.array(sizes)])
    
    if show_output:
        cv2.imshow('Generated', bg_img)
        cv2.waitKey(0)
        cv2.destroyAllWindows()
    return bg_img, centers, sizes

def augment_images_complete_arg(args):
    signs_list, num_of_signs, env, env_paths, show_output = args
    return augment_images_complete(signs_list, num_of_signs, env, env_paths, show_output)
