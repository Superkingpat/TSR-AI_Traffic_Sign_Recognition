from os.path import abspath, dirname, join
DIR = dirname(abspath(__file__))

DATABASE_NAME = "EnvConfig.db"
PATH_TO_SIGN_FOLDER = r"C:\Users\steam\OneDrive\Namizje\test"
PATH_TO_ENV_FOLDER = join(DIR, "Environment")
PATH_TO_GEN_FOLDER = ""

OUTPUT_RESOLUTION = None

NUM_OF_GEN_PHOTOS = 10

CORRECT_COLORS = False
LABEL_PHOTOS = False
SAVE_GEN_LOG = False

# MIN - MAX limits
NUM_OF_SIGNS = (1,5)
ROTATION_2D = (-20,20)
ROTATION_3D = (-20,20)

IGNORE_ENVIRONMENTS = None