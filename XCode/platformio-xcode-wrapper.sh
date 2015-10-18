#!/bin/bash
cd "$PROJECT_DIR"
case $1 in
  clean)
    platformio --force run -t clean
    ;;
  run)
    platformio --force run -t upload
    ;;
  *)
    platformio --force run
    ;;
esac
