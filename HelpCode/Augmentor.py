import Augmentor as ag

if __name__ == "__main__":
    image_directory = r"C:\Data\lableimg\TF_images\predict\crops\Stop"
    output_directory = r"C:\Data\lableimg\TF_images\predict\crops\aug_Stop"
    aug = ag.Pipeline(image_directory, output_directory=output_directory, save_format="JPEG")
    aug.skew(probability=0.7, magnitude=0.7)
    aug.random_distortion(probability=0.6, magnitude=2, grid_width=6, grid_height=6)
    aug.rotate(probability=0.6, max_left_rotation=25, max_right_rotation=25)
    aug.resize(probability=1, width=140, height=140)
    aug.sample(100)