
class CaptureSystem {
        constructor( renderer, sceneRoot, mainCamera, exportCanvas ) {
            this.renderer = renderer;
            this.sceneRoot = sceneRoot;
            this.mainCamera = mainCamera;
            this.previewGroup = null;
            this.timeCaptured = Date.now();
            this.forceCapture = false;
            this.captureStep = -1;
            this.captureStepLast = -1;
            this.exportCanvas = exportCanvas;

            this.savingActiveNow = false;
            

            var camNear = 0.1;
            var camFar = 6.0;
            //this.captureCamera = new THREE.PerspectiveCamera( 90, 1.0, camNear, camFar );
            //this.captureCamera.position.set( 0, 0, 3 );
            this.captureCamera = this.mainCamera;

            const renderSize = new THREE.Vector2();
            renderer.getSize(renderSize)
            var sizeH = renderSize.x; // GNDPacketSocketSizes ? 64 : 480; //16 * 6; // 480 // 16 * 6
            var sizeW = renderSize.y; //GNDPacketSocketSizes ? 64 : 544; //16 * 8; // 544 // 16 * 8
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
        }

}

class ExportSceneSystem {
    static jsonObjectFromThreeCamera(camera) {
        throw "TODO: Next step..."
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

