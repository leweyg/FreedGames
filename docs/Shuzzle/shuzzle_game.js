
var ShuzzleGame_Create = function() {
    var ans = Object.create( ShuzzlePrototype_Game );
    ans.Setup();
    return ans;
};

var ShuzzleThreeJS_Prototype = {
    Game : null,
    SceneRoot : null,
    SceneParent : null,
    Stones : [],
    Cursors : [],
    GameBlocks : [],
    GameGoal : null,
    GameLightHandle : null,
    GameLightSpot : null,
    MatsByType : {},
    RedrawCallback : null,
    DefaultScale : new THREE.Vector3(1,1,0.3),
    Temp1 : new THREE.Vector3(),

    Setup : function(game) {
        this.Game = game;
    },

    Update : function() {

        if (!this.SceneRoot) return;

        for (var bi=0; bi<this.GameBlocks.length; bi++) {
            var blockObj = this.GameBlocks[bi];
            var state = this.Game.State.Core.Blocks[bi];
            blockObj.position.copy( state.Center );
        }

        if (this.GameLightSpot) {

            var bounds = this.Game.Board.Board.Bounds;
            this.GameLightSpot.target.position.set(
                ( bounds.min.x + bounds.max.x ) / 2,
                ( bounds.min.y + bounds.max.y ) / 2,
                ( bounds.min.z - 5 ),
                );

            this.GameLightHandle.position.copy( this.Game.State.Fast.LightPos );
            this.GameLightSpot.position.copy( this.Game.State.Fast.LightPos );

            this.GameLightSpot.updateMatrix();
            this.GameLightSpot.updateMatrixWorld();
            
        }

        for (var ti=0; ti<1; ti++) {
            var cursor = this.Cursors[ti];
            var hover = this.Game.State.Fast.Hovers[ti];
            if (hover >= 0 && hover<this.Stones.length) {
                var over = this.Stones[ hover ];
                cursor.position.copy( over.position );
                cursor.visible = true;
            } else {
                cursor.visible = false;
            }
        }

        if (this.RedrawCallback) {
            this.RedrawCallback();
        }
    },

    BuildBackground : function( bounds ) {
        var trianglePoints = [];
        var triNormals = [];
        var indices = [];
        var pushVertex = ((vert) => {
            trianglePoints.push( vert.x );
            trianglePoints.push( vert.y );
            trianglePoints.push( vert.z );
            triNormals.push( 0 );
            triNormals.push( 0 );
            triNormals.push( 1 );
        });

        var t = new THREE.Vector3();
        var setAnd = ((ox,oy,oz)=>{
            t.x = ox; t.y = oy; t.z = oz; return t;
        });
        bounds = TensorMath.cloneBounds( bounds );
        var edgeSize = 4;
        bounds.min.x -= edgeSize;
        bounds.min.y -= edgeSize;
        bounds.max.x += edgeSize;
        bounds.max.y += edgeSize;
        var base_z = bounds.min.z - 0.5;
        var w = (bounds.max.x - bounds.min.x);
        for (var dy=bounds.min.y; dy<bounds.max.y; dy++) {
            for (var dx=bounds.min.x; dx<bounds.max.x; dx++) {
                var dz = base_z;
                var v = setAnd( dx, dy, dz);
                var edgeDist = TensorMath.boundsMaxAxisDistanceXY( bounds, v );
                if (edgeDist < edgeSize) {
                    var noiseScale = 0.5;
                    dz -= ((edgeDist - edgeSize)*1) + (noiseScale * Math.cos(dx*8 + dy*16));
                    v.z = dz;
                }
                pushVertex( v );

                if ((dx+1 < bounds.max.x) && (dy+1 < bounds.max.y)) {
                    var ix = dx - bounds.min.x;
                    var iy = dy - bounds.min.y;
                    indices.push( (ix+0) + ((iy+0)*w) );
                    indices.push( (ix+1) + ((iy+0)*w) );
                    indices.push( (ix+1) + ((iy+1)*w) );

                    indices.push( (ix+1) + ((iy+1)*w) );
                    indices.push( (ix+0) + ((iy+1)*w) );
                    indices.push( (ix+0) + ((iy+0)*w) );
                }
            }
        }

        var triGeo = new THREE.BufferGeometry();
        triGeo.setIndex( indices );
        triGeo.setAttribute( 'position', new THREE.Float32BufferAttribute( trianglePoints, 3 ) );
        triGeo.setAttribute( 'normal', new THREE.Float32BufferAttribute( triNormals, 3 ) );
        triGeo.computeVertexNormals();
        triGeo.computeBoundingSphere();
        

        var flatMat = new THREE.MeshPhongMaterial( { color: 0x77FF77 } );
        var triMesh = new THREE.Mesh( triGeo, flatMat );

        triMesh.castShadow = false;
        triMesh.receiveShadow = true;

        //triMesh.position.copy( center );
        return triMesh;
    },

    BuildBlock : function( block, state ) {
        
        var center = new THREE.Vector3( 0, 0, 0 );
        center.copy( block.Center );
        if (state) {
            state.Center = center.clone();
        }

        var vpp = block.Mesh.VerticesPerPolygon;
        if (vpp == 2) {
            var linePoints = [];
            for (var vi=0; vi<block.Mesh.Vertices.length; vi++) {
                var pos = block.Mesh.Vertices[vi].pos;
                var loc = new THREE.Vector3( pos.x, pos.y, pos.z );
                loc.add(center);
                linePoints.push( loc );
            }
            const lineMat = new THREE.LineBasicMaterial({
                color: 0x000000,
                //linewidth: 3,
            });
            const lineGeo = new THREE.BufferGeometry().setFromPoints( linePoints );
            const lineObj = new THREE.LineSegments( lineGeo, lineMat );
            lineObj.castShadow = false;
            lineObj.receiveShadow = false;
            return lineObj;
        } else {
            var trianglePoints = [];
            var triNormals = [];
            var pushVertex = ((vert) => {
                trianglePoints.push( vert.pos.x );
                trianglePoints.push( vert.pos.y );
                trianglePoints.push( vert.pos.z );
                triNormals.push( vert.normal.x );
                triNormals.push( vert.normal.y );
                triNormals.push( vert.normal.z );
            });
            var numPolys = block.Mesh.Vertices.length / block.Mesh.VerticesPerPolygon;
            for (var pi=0; pi<numPolys; pi++) {
                for (var ti=1; ti<vpp; ti++) {

                    {
                        var si = (pi*vpp); // zero 
                        pushVertex( block.Mesh.Vertices[si] );
                    }

                    for (var fi=0; fi<2; fi++) {
                        var si = (pi*vpp) + ((ti+fi+0)%vpp); 
                        pushVertex( block.Mesh.Vertices[si] );
                    }
                }
            }

            var triGeo = new THREE.BufferGeometry();
            triGeo.setAttribute( 'position', new THREE.Float32BufferAttribute( trianglePoints, 3 ) );
            triGeo.setAttribute( 'normal', new THREE.Float32BufferAttribute( triNormals, 3 ) );
            triGeo.computeBoundingSphere();

            var defaultColors = [
                0xFF7777, // red
                0x7777FF, // blue
                0xFFff77, // yellow
                0x22FF22, // green
                
                //0xFFff77, // yellow
                0xFF77FF, // purple
            ];
            var color = defaultColors[ block.Index % defaultColors.length ];

            var flatMat = new THREE.MeshPhongMaterial( { 
                color: color,
                shadowSide : THREE.FrontSide } );
            var triMesh = new THREE.Mesh( triGeo, flatMat );

            triMesh.castShadow = true;
            triMesh.receiveShadow = true;

            triMesh.position.copy( center );
            return triMesh;
        } 
    },

    Build : function( contextObj ) {

        if (this.SceneParent && this.SceneRoot) {
            this.SceneParent.remove( this.SceneRoot );
            this.SceneRoot = null;
        }

        this.SceneParent = contextObj;
        var scene = new THREE.Group();
        //scene.quaternion.setFromAxisAngle(new THREE.Vector3(0,0,1),-90);
        this.SceneRoot = scene;

        const bounds = this.Game.Board.Board.Bounds;
        const board_scale = 10;
        this.SceneRoot.scale.set(board_scale,board_scale,board_scale);
        this.SceneRoot.position.set(
            (bounds.min.x + bounds.max.x) * -0.5 * board_scale,
            (bounds.min.y + bounds.max.y) * -0.5 * board_scale,
            -bounds.max.z * board_scale * 1.0);

        this.SceneParent.add( scene );
        
        this.Stones = [];

        this.MatsByType = {};
        this.MatsByType[-1] = new THREE.MeshLambertMaterial( { color: 0xb17c52 } );
        this.MatsByType[ 0] = new THREE.MeshLambertMaterial( { color: 0xffFFff } );
        this.MatsByType[ 1] = new THREE.MeshLambertMaterial( { color: 0x656565 } );

        const piece_scale = 1.0;// * this.Game.Board.NodeScale;

        const sphereGeo = new THREE.SphereBufferGeometry( piece_scale, 12, 12 );

        
        
        var lookTo = new THREE.Vector3();
        var referenceUp = new THREE.Vector3(0,0,1);

        var nodeInfo = this.Game.Board.Board.Blocks;
        this.GameBlocks = [];
        for (var ni=0; ni<nodeInfo.length; ni++) {
            var blockDecl = nodeInfo[ni];
            var state = this.Game.State.Core.Blocks[ni];
            var blockObj = this.BuildBlock( blockDecl, state );
            blockObj.userData.Grab = { type: "block", index:ni  };

            this.GameBlocks.push( blockObj );
            this.SceneRoot.add( blockObj );
        }
        this.GameGoal = this.BuildBlock( this.Game.Board.Board.Goal );
        this.SceneRoot.add( this.GameGoal );

        this.SceneRoot.add( this.BuildBackground( this.Game.Board.Board.Bounds ) );

        var lightMat = new THREE.MeshBasicMaterial( { color: 0xFFff11 } );
        this.GameLightHandle = new THREE.Mesh( sphereGeo, lightMat );
        this.GameLightHandle.position.copy( this.Game.Board.Board.Bounds.max );
        this.GameLightHandle.scale.set(0.3,0.3,0.3);
        this.GameLightHandle.userData.Grab = { type: "light" };
        this.SceneRoot.add( this.GameLightHandle );
        if (true) {
            spotLight = new THREE.SpotLight( 0x808080 );
            spotLight.name = 'Spot Light';
            spotLight.angle = Math.PI / 3;
            spotLight.penumbra = 0.2;
            spotLight.position.set( 10, 10, 5 );
            //spotLight.target = this.SceneRoot;
            spotLight.castShadow = true;
            spotLight.shadow.camera.near = 1.5;
            spotLight.shadow.camera.far = 300;
            spotLight.shadow.mapSize.width = 512;
            spotLight.shadow.mapSize.height = 512;
            spotLight.shadow.focus = 1;
            this.GameLightSpot = spotLight;
            this.SceneRoot.add( spotLight );
            this.SceneRoot.add( spotLight.target );
        }

        if (true) {
            this.Cursors = [ null ];
            for (var ti=0; ti<1; ti++) {
                const cursorObj = new THREE.Mesh( sphereGeo, this.MatsByType[ti] );
                cursorObj.scale.set( 0.5, 0.5, 0.5 );
                this.SceneRoot.add( cursorObj );
                this.Cursors[ti] = cursorObj;
            }
        }

        this.Update();
    }
};

var ShuzzleBoards_Puzzle1 = ShuzzleBoards["puzzle_1"];

var ShuzzlePrototype_Game = {
    Board : ShuzzleBoards_Puzzle1,
    State : ShuzzlePrototype_State,
    Render : ShuzzleThreeJS_Prototype,
    OnCoreUpdated : [],
    OnFastUpdated : [],

    CheckBoardName : function(name) {
        if (this.Board.Name == name) return;
        var to = ShuzzleBoards[name];
        if (!to) {
            console.log("Unknown board '" + name + "'");
            return;
        }

        this.ChangeBoard(to);
    },

    ChangeBoard : function( board ) {
        this.Board = board || ShuzzleBoards_Puzzle1;
        var state = Object.create( ShuzzlePrototype_State );
        this.State = state;
        this.State.Setup( this );
        if (!this.Render) {
            this.Render = Object.create( ShuzzleThreeJS_Prototype );
        }
        this.Render.Setup( this );
        if (this.Render.SceneParent) {
            this.Render.Build( this.Render.SceneParent );
        }

        this.State.OnCoreUpdated.push(() => {
            if (state != this.State) return;
            for (var i in this.OnCoreUpdated) {
                this.OnCoreUpdated[i]();
            }
        });

        this.State.OnFastUpdated.push(() => {
            if (state != this.State) return;
            this.Render.Update();
            for (var i in this.OnFastUpdated) {
                this.OnFastUpdated[i]();
            }
        });

        this.State.DoCoreChangedLocally();
    },

    Setup : function( board ) {
        this.ChangeBoard( board );
        
    },

    OnHoveredOver : function(index) {
        this.State.Fast.Hovers[ this.State.Core.Turn ] = index;
        this.State.DoFastChangedLocally();
    },

    OnLocalUserPassed : function(index) {
        this.State.Core.Turn = ( this.State.Core.Turn + 1 ) % 2;
        this.State.DoCoreChangedLocally();
    },

    OnClickedIndex : function(index) {
        this.State.DoClickedIndex(index);
        
    },

};
