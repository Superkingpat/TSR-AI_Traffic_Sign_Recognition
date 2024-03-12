from kivy.app import App
from kivy.lang import Builder
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.camera import Camera
from kivy.clock import Clock
import time
import os

Builder.load_string(
    """
<CameraClick>:
    orientation: 'vertical'
    Camera:
        id: camera
        resolution: (480, 480)  
        index: 1  # Katero kamero izbrati - ponavadi 0
        play: True  # Kamero avtomatsko zaženi
    Button:
        text: 'Capture'
        size_hint_y: None
        height: '48dp'
        on_press: root.capture()
"""
)


class CameraClick(BoxLayout):
    def __init__(self, **kwargs):
        super(CameraClick, self).__init__(**kwargs)
        self.folder = "captured_images"  # ime folderja
        if not os.path.exists(self.folder):
            os.makedirs(self.folder)  # ustvari folder, če ne obstaja
        # Ustvari image vsakih 0.5 ekund
        Clock.schedule_interval(self.save_frame, 0.5)

    def capture(self):
        """
        Dobi image in jih poimenuje po času
        """
        self.save_frame(0)

    def save_frame(self, dt):
        """
        Funkcija, ki shrani frame in jih doda
        """
        camera = self.ids["camera"]
        timestr = time.strftime("%Y%m%d_%H%M%S")
        filename = os.path.join(self.folder, "IMG_{}.jpeg".format(timestr))
        camera.export_to_png(filename)
        print(f"Captured {filename}")


class TestCamera(App):
    def build(self):
        return CameraClick()


if __name__ == "__main__":
    TestCamera().run()
