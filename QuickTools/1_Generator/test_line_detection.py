from line_detection import load_image, get_random_position, get_n_random_positions

def test_load_image():
    test_image_path = r'QuickTools\1_Generator\test_env.jpg'
    load_image(test_image_path)
    print("load_image function executed successfully.")

def test_get_random_position():
    position = get_random_position()
    print(f"Random position: {position}")

def test_get_n_random_positions():
    positions = get_n_random_positions(5)
    print(f"Random positions: {positions}")

if __name__ == "__main__":
    test_load_image()
    test_get_random_position()
    test_get_n_random_positions()