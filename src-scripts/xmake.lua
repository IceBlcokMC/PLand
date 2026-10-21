-- Independent script targets, making it easy to modify the script at runtime and re-run independent builds
target("scripts")
    set_kind("phony")
    set_default(get_config("scripting"))

    on_load(function (target) 
        import("core.project.config")
        -- register cleanfiles (tsc outdir and qjsc outdir)
        local dist_top = path.join(config.builddir(), ".dist")
        target:add("cleanfiles", path.absolute(dist_top, os.projectdir()))
        local qjsc_top = path.join(config.builddir(), ".qjsc")
        target:add("cleanfiles", path.absolute(qjsc_top, os.projectdir()))
    end)

    on_build(function (target, opt) 
        import("lib.detect.find_tool")
        import("core.project.depend")
        import("utils.progress")
        import("core.project.config")

        local tsc = assert(find_tool("tsc.cmd", { shell = true }), "tsc not found")

        -- TODO: Improve qjsc.exe detection
        -- Current situation:
        -- 1. After the `quickjs-ng` package is installed, its `bin` folder contains `qjsc.exe`. However,
        --     due to xmake's dependency isolation mechanism, it cannot be accessed.
        -- 2. Using a custom package to download a pre-built `qjsc.exe` from a remote repository always
        --     results in xmake throwing an installation error.
        -- 3. Rollback approach: Using `find_tool` first followed by a `curl` download encounters
        --     xmake's caching mechanism, causing subsequent searches to fail upon refresh.
        --
        -- Current workaround: Hardcode a check using `os.exists` to verify file existence, conditionally download
        --  the pre-built artifact from the remote repository via `curl` as needed,
        --   and finally use `find_tool` to bypass the issues mentioned above.
        local qjsc_file = path.join(os.projectdir(), "qjsc.exe")
        if not os.exists(qjsc_file) then
            cprint("${yellow}warning: ${default}qjsc.exe not found, try download...")
            local curl = assert(find_tool("curl.exe"), "curl not found")
            os.vrunv(curl.program, {"-L", "-o", "qjsc.exe", "https://github.com/engsr6982/quickjs-ng/releases/download/v0.15.1/qjsc.exe"})
        end
        local qjsc = assert(find_tool("qjsc.exe", { norun = true }), "qjsc not found")

        -- 1) depend that runs tsc when TS sources change

        local tsconfig_files = {}
        local root_ts = path.join(os.projectdir(), "tsconfig.json")
        if os.isfile(root_ts) then
            table.insert(tsconfig_files, root_ts)
        end

        local sourcefiles = os.files("src-scripts/**.ts")
        local tsc_inputs = table.join(sourcefiles, tsconfig_files)

        local js_outdir = path.absolute(path.join(config.builddir(), ".dist"), os.projectdir())

        local tsc_stamp = path.join(js_outdir, ".tsc.stamp") -- guard file
        table.insert(tsc_inputs, tsc_stamp)

        depend.on_changed(function ()
            os.mkdir(js_outdir)
            progress.show(opt.progress, "${color.build.target}build.tsc %s", "src-scripts")
            os.vrunv(tsc.program, {}, { shell = true })
            io.writefile(tsc_stamp, "ok\n")
        end, {
            files = tsc_inputs,
            dependfile = target:dependfile(tsc_stamp),
            changed = target:is_rebuilt()
        })

        -- 2) depend that scans JS outputs and runs qjsc when JS files changed
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
target_end()
