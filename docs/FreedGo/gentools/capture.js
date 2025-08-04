
class CaptureSystem {
        constructor( renderer, sceneRoot, mainCamera, exportCanvas ) {
            this.renderer = renderer;
            this.sceneRoot = sceneRoot;
            this.mainCamera = mainCamera;
            this.exportCanvas = exportCanvas;

            this.savingActiveNow = false;
            

            var camNear = 0.1;
            var camFar = 6.0;
            //this.captureCamera = new THREE.PerspectiveCamera( 90, 1.0, camNear, camFar );
            //this.captureCamera.position.set( 0, 0, 3 );
            this.captureCamera = this.mainCamera;

            const renderSize = new THREE.Vector2();
            renderer.getSize(renderSize)
            var sizeH = renderSize.x;
            var sizeW = renderSize.y;
            this.sizeH = sizeH;
            this.sizeW = sizeW;
            this.renderTarget = new THREE.WebGLRenderTarget( sizeW, sizeH );
            this.renderTarget2 = new THREE.WebGLRenderTarget( sizeW, sizeH );
            this.outTexture = this.renderTarget.texture;

            this.readImageBuffer = new ImageData(sizeW, sizeH);
            this.readBufferSize = sizeW * sizeH * 4;
            this.readBufferData = this.readImageBuffer.data;
            this.readImageBuffer2 = new ImageData(sizeW, sizeH);
            this.readBufferData2 = this.readImageBuffer2.data;
            this.readCanvas = document.createElement('canvas');
            this.readCanvas.width = sizeW;
            this.readCanvas.height = sizeH;
        }

        doTest() {
            console.log("doTest...");
            CaptureFileSystem.GetPWD((res)=>{
                alert("Pwd=" + res);
            })
            console.log("Done...");
        }

        doExportNow(viewId) {
            const exportPrefix = "gentools/view_" + viewId + "_";
            var renderer = this.renderer;
            renderer.setRenderTarget( this.renderTarget );
            renderer.clear();
            renderer.render( this.sceneRoot, this.captureCamera );

            var camObj = this.captureCamera;

            renderer.setRenderTarget(null);
            this._doExportRenderTarget(this.renderTarget, exportPrefix, this.exportCanvas, camObj);
        }

        async _doExportRenderTarget(target,exportPrefix,canvas,camInfo) {
            const save_path_img = exportPrefix + "color.png";
            const save_path_json = exportPrefix + "scene.json";
            const save_path_img_relative = save_path_img.substring(save_path_img.lastIndexOf("/")+1);
                this.renderer.readRenderTargetPixels(target,
                        0, 0, this.sizeW, this.sizeH, this.readBufferData);

                /*
                var imgTensor = {
                        data : this.readBufferData,
                        dtype: "uint8",
                        shape : [
                                { name:"h", size:this.sizeH },
                                { name:"w", size:this.sizeW },
                                { name:"rgba", size:4 }
                        ]
                };
                */

                //const canvas = this.exportCanvas;
                const ctx = canvas.getContext("2d");
                ctx.putImageData(this.readImageBuffer, 0, 0);
                
                canvas.toBlob((imgBlob) => {
                        CaptureFileSystem.SaveFileContent(save_path_img, imgBlob, (res)=>{
                            //alert("Post Responce=" + res);
                        });
                });

                var objScene = {root:null, objects:{}, tensors:{}}
                var obj = ExportSceneSystem.jsonObjectFromThreeElementRecursive(this.sceneRoot, objScene)
                var camObj = ExportSceneSystem.jsonObjectFromThreeCamera(camInfo, save_path_img_relative);
                obj.children.push(camObj)
                ExportSceneSystem.jsonObjectProjectedInfo(this.sceneRoot, camInfo, objScene);
                objScene.root = obj;
                var txt = ExportSceneSystem.textFromJsonObject(objScene);
                CaptureFileSystem.SaveFileContent(save_path_json, txt, (res)=>{
                    //alert("Post Responce=" + res);
                });
        }

}

class ExportSceneSystem {
    static textFromJsonObject(obj) {
        return JSON.stringify(obj,null,2);
    }
    static gRefIdentity = null;
    static isMatrix4Identity(mat) {
        if (!this.gRefIdentity) {
            this.gRefIdentity = new THREE.Matrix4();
            this.gRefIdentity.identity();
        }
        return mat.equals(this.gRefIdentity);
    }
    static jsonObjectProjectedInfo(worldRoot, camera, objScene) {
        const ans = {
                shape:[0,2,3], // fill out later
                data:[],
                dtype:"number",
        }
        var pos = new THREE.Vector3();
        var rows = 0;
        for (var bi in worldRoot.children) {
            const board = worldRoot.children[bi];
            if (board.name != "board") continue;
            for (var si in board.children) {
                const stone = board.children[si];
                if (!stone.name.startsWith("stone")) continue;
                pos.setFromMatrixPosition(stone.matrixWorld);
                ans.data.push(pos.x, pos.y, pos.z);
                pos.project(camera)
                ans.data.push(pos.x, pos.y, pos.z);
                rows++;
            }
        }
        ans.shape[0] = rows;

        objScene.tensors["projected_points"] = ans;
        return ans;
    }
    static dataArrayFromAttribute3(attrib) {
        const v = new THREE.Vector3();
        const ans = []
        for (let i = 0; i < attrib.count; i++) {
            v.fromBufferAttribute(attrib, i);
            ans.push(v.x)
            ans.push(v.y)
            ans.push(v.z);
        }
        return ans;
    }
    static dataArrayFromAttribute1(attrib) {
        const v = new THREE.Vector3();
        const ans = []
        for (let i = 0; i < attrib.count; i++) {
            v.fromBufferAttribute(attrib, i);
            ans.push(v.x)
            ans.push(v.y)
            ans.push(v.z);
        }
        return ans;
    }
    static meshStoneName() {
        return "mesh_stone";
    }
    static meshStoneObject(meshElement) {
        var ans = {
            name:this.meshStoneName(),
        }
        const geo = meshElement.geometry;
        const poses = geo.getAttribute("position");
        ans.content = {
            vertices:{
                shape:[poses.count, poses.itemSize],
                data:this.dataArrayFromAttribute3(poses),
                dtype:"number"
            },
            indices:{
                data:geo.index.array,
                shape:[geo.index.count/3, 3],
                dtype:"uint16",
            },
        };
        return ans;
    }
    static jsonObjectFromThreeElementRecursive(el, outScene) {
        const ans = {};
        if (el.name) {
            ans.name = el.name;
        }
        if (el.matrix && !this.isMatrix4Identity(el.matrix)) {
            ans.pose = this.fromThreeMatrix(el.matrix);
        }
        if (el.children && el.children.length) {
            ans.children = [];
            for (var ci in el.children) {
                const child = el.children[ci];
                const cobj = this.jsonObjectFromThreeElementRecursive(child, outScene);
                ans.children.push(cobj)
            }
        }
        if (el.isMesh) {
            if (!ans.children) {
                ans.children = []
            }
            const meshObjName = this.meshStoneName()
            if (!(meshObjName in outScene.objects)) {
                outScene.objects[meshObjName] = this.meshStoneObject(el);
            }
            ans.children.push(meshObjName)
        }
        return ans;
    }
    static jsonObjectFromThreeCamera(camera, image_path) {
        const image = {
            name:"image",
            content:{path:image_path},
            unpose:"pixel_index_from_unit_viewport",
            
        }
        const lens = {
            name: "lens",
            children : [image],
            unpose:this.fromThreeMatrix(camera.projectionMatrix),
            
        };
        var placed = {
            name: camera.name ? camera.name : "camera",
            children: [lens],
            pose:this.fromThreeMatrix(camera.matrix),
            
        }
        console.assert(!camera.parent) // TODO: support later on
        for (var par = camera.parent; par; par = par.parent) {
            par_placed = {
                name: par.name,
                children: [placed],
                pose: this.fromThreeMatrix(par.matrix),
                
            }
            placed = par_placed;
        }
        return placed;
    }
    static fromThreeMatrix(mat) { // mat : THREE.Matrix4
        return {
            shape:[4,4],
            dtype:"number",
            data:mat.elements,
        }
    }
}

class CaptureFileSystem {
        static SaveFileContent(path,content,callback,folderRoot="") {
            console.log("Saving to '" + path + "'...");
                var url = folderRoot + "gentools/save_to_file.php?path=" + path;
                if (path.includes("/")) {
                        var dir = path.substring(0,path.lastIndexOf("/"));
                        url += "&dir=" + dir;
                }
                var rawFile = new XMLHttpRequest();
                rawFile.overrideMimeType("application/json");
                rawFile.open("POST", url, true);
                rawFile.onreadystatechange = function() {
                        if (rawFile.readyState === 4 && rawFile.status == "200") {
                        if (callback) {
                                callback(rawFile.responseText);
                        }
                        console.log("Saved content to '" + path + "'.");
                        }
                }
                rawFile.send(content);
        }
        static GetPWD(callback) {
                var url = "gentools/pwd.php";
                var rawFile = new XMLHttpRequest();
                rawFile.overrideMimeType("text/plain");
                rawFile.open("GET", url, true);
                rawFile.onreadystatechange = function() {
                        if (rawFile.readyState === 4 && rawFile.status == "200") {
                        if (callback) {
                                callback(rawFile.responseText);
                        }
                        console.log("Done calling pwd:" + rawFile.responseText);
                        }
                }
                rawFile.send()
        }
        
}

