add_rules("mode.debug", "mode.release")

add_repositories("levimc-repo https://github.com/LiteLDev/xmake-repo.git")
add_repositories("iceblcokmc https://github.com/IceBlcokMC/xmake-repo.git")
add_repositories("engsr6982-repo https://github.com/engsr6982/xmake-repo.git")

-- LeviMc(LiteLDev)
add_requires("levilamina 26.51.5", {configs = {target_type = "server"}})
add_requires("levibuildscript main")
add_requires("ilistenattentively 0.17.0")

-- IceBlockMC
add_requires("ll-bstats 0.7.0")
add_requires("economy_bridge 0.7.0")

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
if has_config("scripting") then
    add_requires("jspp b259bdaf2182b3a7b7fb84d0c0fd1ddf98b00bc1")
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

option("scripting")
    set_default(true)
    set_showmenu(true)


--------------------------------------------------------------------------
--
--------------------------------------------------------------------------

function apply_common_flags()
    set_policy("check.target_package_licenses", false)

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
    add_defines("LDAPI_EXPORT", "LL_PLAT_S")

    add_packages(
        "levilamina",
        "exprtk",
        "ilistenattentively",
        "ll-bstats",
        "economy_bridge",
        "abseil"
    )

    if is_mode("debug") then
        add_defines("PLAND_DEBUG")
        --add_defines(
        --    "PLAND_I18N_COLLECT_STRINGS",
        --    "LL_I18N_COLLECT_STRINGS",
        --    "LL_I18N_COLLECT_STRINGS_CUSTOM",
        --    "LL_I18N_STRING_LITERAL_TYPE=::ll::FixedString"
        --)
    end

    -- Subcomponent switches, shared by every target (src/PLand.cpp compiles the lifecycle glue
    -- behind these ifdefs, so the defines must be present while compiling core too, otherwise
    -- PLand::getScriptingBootstrap() etc. are never emitted and the final link fails).
    -- Only clean pimpl headers (ScriptingBootstrap.h / DevToolApp.h) are pulled in from the
    -- sibling dirs, so jspp/imgui updates still never recompile core.
    if get_config("devtool") then
        add_defines("LD_DEVTOOL")
        add_includedirs("src-devtool")
    end
    if get_config("scripting") then
        add_defines("PLAND_SCRIPTING")
        add_includedirs("src-scripting")
    end
end

-- Parse the latest git tag into PLAND_VERSION_* configvars on `target`; returns the version string.
-- Shared by core-obj (generates src/pland/_version.h) and PLand (modpacker metadata).
-- NOTE: must stay a `local` function: on_load callbacks run in the runtime sandbox and can only
-- reach description-scope names through closure upvalues, not globals.
local function resolve_pland_version(target)
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
    return versionStr
end

target("core-obj")
    set_kind("object")

    apply_common_flags()

    add_includedirs("src", {public = true})
    add_files("src/**.cpp", "src/**.cc")
    add_headerfiles("src/(pland/**.h)")

    -- Generate src/pland/_version.h here: core sources (PLand.cpp, Telemetry.cc) include it and
    -- core-obj builds before the final PLand target would generate it.
    set_configvar("BUILD_VARIANT", get_config("devtool") and "devtool" or "headless")
    set_configvar("HEADLESS", get_config("devtool") and "true" or "false")
    add_configfiles("src/_version.h.in")
    set_configdir("src/pland")

    on_load(resolve_pland_version)
target_end()


target("devtool-obj")
    set_kind("object")
    add_deps("core-obj")
    set_default(get_config("devtool"))

    apply_common_flags()

    add_files("src-devtool/**.cc")

    add_packages(
        "imgui",
        "glew",
        "imgui_color_text_edit",
        {public = true}
    )
target_end()

target("scripting-obj")
    set_kind("object")
    add_deps("core-obj")
    set_default(get_config("scripting"))

    apply_common_flags()

    add_files("src-scripting/**.cc")

    add_packages("jspp", {public = true})
target_end()

target("PLand")
    add_rules("@levibuildscript/linkrule")
    add_rules("plugin.compile_commands.autoupdate")
    -- FIX: after splitting sources into pland-*-obj targets, this target only compiles
    -- BinaryMeta.win.rc, so xmake builds its linker from the `msrc` language, whose
    -- nameflags have no target.links/linkdirs/shflags entries -> every package/dep
    -- library silently disappeared from the link command. Attaching the c++ rule
    -- (it declares set_sourcekinds("cxx")) restores a c++ linker with full nameflags.
    add_rules("c++")

    set_kind("shared")
    add_deps("core-obj")
    if get_config("devtool") then
        add_deps("devtool-obj")
    end
    if get_config("scripting") then
        add_deps("scripting-obj")
    end

    apply_common_flags()

    if is_plat("windows") then
        add_files("src/BinaryMeta.win.rc")
    end

    on_load(function (target)
        local versionStr = resolve_pland_version(target)

        target:add("rules", "@levibuildscript/modpacker",{
            modName = target:basename(),
            modVersion = versionStr
        })
    end)

    after_build(function (target)
        local projectdir = os.projectdir()
        local bindir = path.join(projectdir, "bin")
        local outputdir = path.join(bindir, target:name())

        local assetsdir = path.join(projectdir, "assets")
        local langDir = path.join(assetsdir, "lang")
        os.mkdir(path.join(outputdir, "lang"))
        os.cp(langDir, outputdir)
    end)
target_end()

package("jspp")
    set_urls("https://github.com/engsr6982/jspp.git")

    add_configs("backend", {default = "QuickJs", values = {"QuickJs", "V8"}})

    add_deps("cmake")
    add_deps("quickjs-ng v0.15.1")

    on_load(function (package)
        package:add("defines", "JSPP_BACKEND_QUICKJS")
        if package:debug() then
            package:add("defines", "JSPP_DEBUG")
        end
    end)

    on_install(function (package)
        local configs = { "-DJSPP_BACKEND=quickjs" }

        local pkg = package:dep("quickjs-ng")
        if pkg then
            local installdir = pkg:installdir()
            local incdir = path.join(installdir, "include")
            local libdir = path.join(installdir, "lib")

            table.insert(configs, "-DJSPP_EXTERNAL_INC=" .. incdir)
            table.insert(configs, "-DJSPP_EXTERNAL_LIB=" .. libdir)
        end

        import("package.tools.cmake").install(package, configs)
    end)
package_end()

includes("src-scripts")
