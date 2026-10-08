import {clearInterval, log, setInterval, setTimeout} from "@runtime"
import {type CommandOrigin, Player} from "@minecraft"
import {CommandParamKind, CommandRegistrar} from "@levilamina";

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

    const cmd = registrar.getOrCreateCommand("paxxx")

    let ori: CommandOrigin | null = null;
    cmd.runtimeOverload()
        .text("a")
        .required("enu", CommandParamKind.Enum, lenumn)
        .required("str", CommandParamKind.String)
        .execute((origin, output, args) => {
            log("origin.type={}", origin.originType);
            output.success("args.enu={}, args.str={}", args.enu, args.str);
            ori = origin;
        })

    cmd.runtimeOverload().text("scripts").text("b").required("str", CommandParamKind.String).execute((origin, output, args) => {
        output.error("type={}", ori?.originType);
    })
}