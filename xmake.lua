add_rules("mode.debug", "mode.release")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")
add_repositories("iceblcokmc https://github.com/IceBlcokMC/xmake-repo.git")
add_repositories("engsr6982-repo https://github.com/engsr6982/xmake-repo.git")

-- LeviMc(LiteLDev)
add_requires("levilamina 26.40.6", {configs = {target_type = "server"}})
add_requires("levibuildscript")
add_requires("ilistenattentively 0.16.0")

-- IceBlockMC
add_requires("ll-bstats 0.6.0")
add_requires("economy_bridge 0.6.0")

-- xmake
add_requires("exprtk 0.0.3")
add_requires("abseil 20250127.0")

if has_config("devtool") then
    -- xmake
    add_requires("imgui v1.92.7-docking", {configs = { opengl3 = true, glfw = true }})
    add_requires("glew 2.2.0")

    -- engsr6982
    add_requires("imgui_color_text_edit")
end


if not has_config("vs_runtime") then
    set_runtimes("MD")
end

if is_plat("windows") then
    set_toolchains("clang-cl") -- windows allways use clang-cl
end

option("devtool") -- 开发工具
    set_default(true)
    set_showmenu(true)
option_end()

target("PLand")
    add_rules("@levibuildscript/linkrule")
    add_rules("plugin.compile_commands.autoupdate")
    set_kind("shared")
    set_languages("c++20")
    set_symbols("debug")
    if is_plat("windows") then
        add_defines("NOMINMAX", "UNICODE")
        set_exceptions("cxx")
        add_cxflags("/utf-8", "/W4", "/w44265", "/w44289", "/w44296", "/w45263", "/w44738", "/w45204")
        add_cxflags(
            "/EHs",
            "-Wno-microsoft-cast",
            "-Wno-invalid-offsetof",
            "-Wno-c++2b-extensions",
            "-Wno-microsoft-include",
            "-Wno-overloaded-virtual",
            "-Wno-ignored-qualifiers",
            "-Wno-missing-field-initializers",
            "-Wno-potentially-evaluated-expression",
            "-Wno-pragma-system-header-outside-header",
            {tools = {"clang_cl"}}
        )
    end
    add_defines(
        "LDAPI_EXPORT",
        "LL_PLAT_S"
    )
    add_includedirs("src")
    add_files("src/**.cpp", "src/**.cc")
    add_headerfiles("src/(pland/**.h)")

    add_packages(
        "levilamina",
        "exprtk",
        "ilistenattentively",
        "ll-bstats",
        "economy_bridge",
        "abseil"
    )

    set_configvar("BUILD_VARIANT", get_config("devtool") and "devtool" or "headless")
    set_configvar("HEADLESS", get_config("devtool") and "true" or "false")
    add_configfiles("src/_version.h.in")
    set_configdir("src/pland")

    if is_mode("debug") then
        add_defines("PLAND_DEBUG")
        -- add_defines("PLAND_I18N_COLLECT_STRINGS", "LL_I18N_COLLECT_STRINGS", "LL_I18N_COLLECT_STRINGS_CUSTOM")
    end

    if is_plat("windows") then
        add_files("src/BinaryMeta.win.rc")
    end

    if get_config("devtool") then
        add_packages(
            "imgui",
            "glew",
            "imgui_color_text_edit"
        )
        add_includedirs("src-devtool")
        add_files("src-devtool/**.cc")
        add_defines("LD_DEVTOOL")
    end

    on_load(function (target)
        local tag = os.iorun("git describe --tags --abbrev=0 --always")
        local major, minor, patch, suffix = tag:match("v(%d+)%.(%d+)%.(%d+)(.*)")
        if not major then
            print("Failed to parse version tag, using 0.0.0")
            major, minor, patch = 0, 0, 0
        end
        local versionStr =  major.."."..minor.."."..patch
        if suffix then
            prerelease = suffix:match("-(.*)")
            if prerelease then
                prerelease = prerelease:gsub("\n", "")
            end
            if prerelease then
                target:set("configvar", "PLAND_VERSION_PRERELEASE", prerelease)
                versionStr = versionStr.."-"..prerelease
            end
        end
        target:set("configvar", "PLAND_VERSION_MAJOR", major)
        target:set("configvar", "PLAND_VERSION_MINOR", minor)
        target:set("configvar", "PLAND_VERSION_PATCH", patch)

        target:add("rules", "@levibuildscript/modpacker",{
            modName = target:basename(),
            modVersion = versionStr
        })
    end)

    after_build(function (target)
        local bindir = path.join(os.projectdir(), "bin")
        local outputdir = path.join(bindir, target:name())

        local assetsdir = path.join(os.projectdir(), "assets")
        local langDir = path.join(assetsdir, "lang")
        os.mkdir(path.join(outputdir, "lang"))
        os.cp(langDir, outputdir)
    end)
