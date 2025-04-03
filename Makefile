PREFIX ?= /usr/local
BUILD_DIR = ./build
EXECUTABLE = rclone-browser
ICONS = \
	share/icons/hicolor/scalable/apps/rclone-browser.svg \
	share/icons/hicolor/32x32/apps/rclone-browser.png \
	share/icons/hicolor/64x64/apps/rclone-browser.png \
	share/icons/hicolor/128x128/apps/rclone-browser.png \
	share/icons/hicolor/256x256/apps/rclone-browser.png \
	share/icons/hicolor/512x512/apps/rclone-browser.png
DESKTOP_FILE = share/applications/rclone-browser.desktop

all:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. && make

install:
	cd $(BUILD_DIR) && make install

uninstall:
	rm -f $(PREFIX)/bin/$(EXECUTABLE)
	rm -f $(PREFIX)/share/icons/hicolor/scalable/apps/rclone-browser.svg
	rm -f $(PREFIX)/share/icons/hicolor/32x32/apps/rclone-browser.png
	rm -f $(PREFIX)/share/icons/hicolor/64x64/apps/rclone-browser.png
	rm -f $(PREFIX)/share/icons/hicolor/128x128/apps/rclone-browser.png
	rm -f $(PREFIX)/share/icons/hicolor/256x256/apps/rclone-browser.png
	rm -f $(PREFIX)/share/icons/hicolor/512x512/apps/rclone-browser.png
	rm -f $(PREFIX)/share/applications/rclone-browser.desktop

clean:
	rm -rf $(BUILD_DIR)
