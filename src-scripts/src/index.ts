import {clearInterval, log, setInterval, setTimeout} from "@runtime"
import {type CommandOrigin, Player} from "@minecraft"
import {CommandParamKind, CommandRegistrar, CustomForm, ModalForm, SimpleForm} from "@levilamina";

function launchTask() {

    setTimeout(() => log("timeout called"), 1000);

    let clear = false
    let count = 0;
    const id = setInterval(() => {
        if (count < 3) {
            count++;
            log("setInterval called, count = {}", count);
        } else {
            if (clear == true) {
                throw new Error("clear interval failed");
            }
            clear = true;
            const ok = clearInterval(id);
            log("clearInterval called, res={}", ok);
            if (!ok) {
                throw new Error("clear interval failed");
            }
        }
    }, 1000)
}

export default class ScriptMod {
    constructor() {
    }

    onLoad() {
        log("onLoad called");
        log("onLoad called, {}", 123);
    }

    onEnable() {
        log("onEnable called");

        launchTask();

        setupCommand();
    }

    onDisable() {
        log("onDisable called");
    }
}

function setupCommand() {
    let registrar = CommandRegistrar.getInstance();

    const lenumn = "asd";
    const lenum = [
        ["op1", 0],
        ["op2", 1],
    ]
    if (!registrar.hasEnum(lenumn)) {
        registrar.tryRegisterRuntimeEnum(lenumn, lenum as pair<string, number>[]);
    }

    const cmd = registrar.getOrCreateCommand("pland")
    cmd.runtimeOverload().text("scripts").text("custom_f").execute((origin, output, args) => {
        if (!origin.player) {
            output.error("player not found!");
            return;
        }
        let f = new CustomForm();
        f.appendLabel("test")
            .appendDivider()
            .appendInput("aa", "str", "placl", "def", "tip")
            .sendTo(origin.player, (player, result, cancelReason) => {
                log("result={}", result?.aa)
            })
    })
    cmd.runtimeOverload().text("scripts").text("modal_f").execute((origin, output, args) => {
        if (!origin.player) {
            output.error("player not found!");
            return;
        }
        new ModalForm()
            .setTitle("modal")
            .setContent("aaa")
            .setUpperButton("up")
            .setLowerButton("down")
            .sendTo(origin.player, (player, result, cancelReason) => {
                log("result={}", result);
            })
    });
    cmd.runtimeOverload().text("scripts").text("simple_f").execute((origin, output, args) => {
        if (!origin.player) {
            output.error("player not found!");
            return;
        }
        new SimpleForm()
            .setTitle("simple_f")
            .appendButton("a", null)
            .appendButton("b", (player) => {
                log("b");
            })
            .sendTo(origin.player, null);
    });

}