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
    add_requires("jspp 91202048f35151e156ccf8e00b3412dc94b7d9a3")
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
        --add_defines(
        --    "PLAND_I18N_COLLECT_STRINGS",
        --    "LL_I18N_COLLECT_STRINGS",
        --    "LL_I18N_COLLECT_STRINGS_CUSTOM",
        --    "LL_I18N_STRING_LITERAL_TYPE=::ll::FixedString"
        --)
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

    if get_config("scripting") then
        add_packages("jspp")
        add_includedirs("src-scripting")
        add_files("src-scripting/**.cc")
        add_defines("PLAND_SCRIPTING")
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
        local projectdir = os.projectdir()
        local bindir = path.join(projectdir, "bin")
        local outputdir = path.join(bindir, target:name())

        local assetsdir = path.join(projectdir, "assets")
        local langDir = path.join(assetsdir, "lang")
        os.mkdir(path.join(outputdir, "lang"))
        os.cp(langDir, outputdir)
    end)

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


-- src-scripts --TS--> Node.js --tsc--> qjs.exe --bin--> build --after_build--> bin/PLand/scripts
rule("bytecode")
    set_extensions(".ts")

    on_load(function (target)
        import("core.project.config")
        -- register cleanfiles (tsc outdir and qjsc outdir)
        local dist_top = path.join(config.builddir(), ".dist")
        target:add("cleanfiles", path.absolute(dist_top, os.projectdir()))
        local qjsc_top = path.join(config.builddir(), ".qjsc")
        target:add("cleanfiles", path.absolute(qjsc_top, os.projectdir()))
    end)

    on_build_files(function (target, sourcebatch, opt)
        import("lib.detect.find_tool")
        import("core.project.depend")
        import("utils.progress")
        import("core.project.config")

        local tsc = assert(find_tool("tsc.cmd", { shell = true }), "tsc not found")
        local qjsc = assert(find_tool("qjsc.exe", { norun = true }), "qjsc not found")

        -- 1) depend that runs tsc when TS sources change

        local tsconfig_files = {}
        local root_ts = path.join(os.projectdir(), "tsconfig.json")
        if os.isfile(root_ts) then
            table.insert(tsconfig_files, root_ts)
        end

        local tsc_inputs = table.join(sourcebatch.sourcefiles, tsconfig_files)

        depend.on_changed(function ()
            progress.show(opt.progress, "${color.build.target}build.tsc %s", "src-scripts")
            os.vrunv(tsc.program, {}, { shell = true })
        end, {
            files = tsc_inputs,
            changed = target:is_rebuilt()
        })

        -- 2) depend that scans JS outputs and runs qjsc when JS files changed
        local js_outdir = path.absolute(path.join(config.builddir(), ".dist"), os.projectdir())
        local jsfiles = os.files(path.join(js_outdir, "**.js")) or {}

        -- scan all native module, avoid qjsc resolve it failed
        local scan_files = os.files("src-scripts/types/**.d.ts") or {}
        local scan_seen, ignore_module_args = {}, {}
        local scan_pattern = "^%s*declare%s+module%s+['\"](.-)['\"]"
        for _, f in ipairs(scan_files) do
            local content = io.readfile(f)
            if content then
                for line in content:gmatch("[^\r\n]+") do
                    local name = line:match(scan_pattern)
                    if name and not scan_seen[name] then
                        scan_seen[name] = true
                        table.insert(ignore_module_args, "-M")
                        table.insert(ignore_module_args, name)
                    end
                end
            end
        end

        local bin_outdir = path.absolute(path.join(config.builddir(), ".qjsc"), os.projectdir());
        os.mkdir(bin_outdir)
        for _, js in ipairs(jsfiles) do
            local out_name = path.basename(js) .. ".bin";
            local outfile = path.join(bin_outdir, out_name)

            depend.on_changed(function ()
                local rel_js = path.relative(js, os.projectdir()) or js
                progress.show(opt.progress, "${color.build.target}build.qjsc %s", rel_js)

                -- run qjsc in js's directory so relative imports resolve
                local oldir = os.cd(path.directory(js))

                local args = table.join(ignore_module_args, {"-b", "-o", outfile, path.filename(js)})
                os.vrunv(qjsc.program, args)

                os.cd(oldir)
            end, {
                files = { js },
                lastmtime = os.mtime(outfile),
                changed = target:is_rebuilt()
            })
        end
    end)

target("scripts")
    set_kind("object")
    set_default(get_config("scripting"))
    add_files("src-scripts/**.ts", { rule = "bytecode" })

    --copy build/.qjsc into bin/PLand/scripts
    after_build(function(target)
        local projectdir = os.projectdir()
        local bindir = path.join(projectdir, "bin")
        local outputdir = path.join(bindir, "PLand", "scripts")

        os.mkdir(outputdir)

        local builddir = path.join(projectdir, "build")
        local qjscdir = path.join(builddir, ".qjsc/*")
        os.cp(qjscdir, outputdir)
    end)