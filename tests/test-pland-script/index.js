import { logger, LogLevel } from "@levilamina";

export default class Entry {
  onLoad() {
    logger.setLevel(LogLevel.Debug);
    logger.info("onLoad called");
  }

  onEnable() {
    logger.info("onEnable called");
  }

  onDisable() {
    logger.info("onDisable called");
  }
}
