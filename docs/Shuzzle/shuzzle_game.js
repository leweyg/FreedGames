
var ShuzzleGame_Create = function() {
    var ans = Object.create( ShuzzlePrototype_Game );
    ans.Setup();
    return ans;
};

var ShuzzleSaveCallbacks = {
    DoSaveCore : ((core) => {
        //console.log("TODO: Save core");
    }),
    DoSaveFast : ((fast) => {
        //console.log("TODO: Save core");
    }),
};

var ShuzzlePrototype_State = {
    Game : null,
    Fast : {
        Hovers : [],
    },
    Core : {
        BoardName : "mobius",
        Blocks : [],
    },
    OnCoreUpdated : [],
    OnFastUpdated : [],

    Setup : function(game) {
        this.Game = game;
        this.NewBoard();
    },

    PreCoreSave : function() {
        this.Core.BoardName = this.Game.Board.Name;
    },

    NewBoard : function() {
        var board = this.Game.Board.Board;
        var numNodes = board.Blocks.length;
        this.Core.BoardName = this.Game.Board.ShuzzleLevel;
        this.Core.Blocks = []; 
        this.Core.Blocks.length = numNodes;
        this.Fast.Hovers = [ -1, -1 ];
        for (var i=0; i<numNodes; i++) {
            this.Core.Blocks[i] = { 
                Center : {x:0, y:0, z:0},
                Rotation : {
                    x:{x:1}, y:{y:1}, z:{z:1}
                }
            };
        }
    },

    DoFastChangedRemote : function() {
        for (var i in this.OnFastUpdated) {
            this.OnFastUpdated[i]();
        }
    },

    DoFastChangedLocally : function() {
        this.DoFastChangedRemote();
        // and now publish the state... (save)
        ShuzzleSaveCallbacks.DoSaveFast( this.Fast );
    },

    DoCoreChangedRemote : function() {
        if (this.Core.BoardName != this.Game.Board.Name) {
            this.Game.Board 
        }
        for (var i in this.OnCoreUpdated) {
            this.OnCoreUpdated[i]();
        }
        this.DoFastChangedLocally();
    },

    DoCoreChangedLocally : function() {
        this.DoCoreChangedRemote();
        // and now publish the state... (save)
        ShuzzleSaveCallbacks.DoSaveCore( this.Core );
    },

    DoClickedIndex : function(index) {
        if ((index < 0) || (index >= this.Core.Nodes.length)) {
            return false;
        }
        var cur = this.Core.Nodes[ index ];
        var to = this.Core.Turn;
        if (cur >= 0) {
            to = -1;
            var otherTurn = ((cur + 1)%2);
            this.Core.Taken[otherTurn]++;
        }
        if (this.DoPlaceStone( index, to )) {
            this.Core.Turn = ((this.Core.Turn + 1)%2);
        }
        this.DoCoreChangedLocally();
    }

};

var ShuzzleThreeJS_Prototype = {
    Game : null,
    SceneRoot : null,
    SceneParent : null,
    Stones : [],
    Cursors : [],
    GameBlocks : [],
    GameGoal : null,
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
        var dz = bounds.min.z - 0.5;
        for (var dx=bounds.min.x; dx<bounds.max.x; dx++) {
            for (var dy=bounds.min.y; dy<bounds.max.y; dy++) {
                pushVertex( setAnd( dx, dy, dz) );
                pushVertex( setAnd( dx+1, dy, dz) );
                pushVertex( setAnd( dx+1, dy+1, dz) );

                pushVertex( setAnd( dx+1, dy+1, dz) );
                pushVertex( setAnd( dx, dy+1, dz) );
                pushVertex( setAnd( dx, dy, dz) );
            }
        }

        var triGeo = new THREE.BufferGeometry();
        triGeo.setAttribute( 'position', new THREE.Float32BufferAttribute( trianglePoints, 3 ) );
        triGeo.setAttribute( 'normal', new THREE.Float32BufferAttribute( triNormals, 3 ) );
        triGeo.computeBoundingSphere();

        var flatMat = new THREE.MeshLambertMaterial( { color: 0x7777FF } );
        var triMesh = new THREE.Mesh( triGeo, flatMat );

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
                0xFF7777,
                0x77FF77,
                //0x7777FF,
                0xFFff77,
                0xFF77FF,
            ];
            var color = defaultColors[ block.Index % defaultColors.length ];

            var flatMat = new THREE.MeshLambertMaterial( { color: color } );
            var triMesh = new THREE.Mesh( triGeo, flatMat );

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

        const board_scale = 10;
        this.SceneRoot.scale.set(board_scale,board_scale,board_scale);

        this.SceneParent.add( scene );
        
        this.Stones = [];

        this.MatsByType = {};
        this.MatsByType[-1] = new THREE.MeshLambertMaterial( { color: 0xb17c52 } );
        this.MatsByType[ 0] = new THREE.MeshLambertMaterial( { color: 0xffFFff } );
        this.MatsByType[ 1] = new THREE.MeshLambertMaterial( { color: 0x656565 } );

        const piece_scale = 26;// * this.Game.Board.NodeScale;

        const geometry = new THREE.SphereBufferGeometry( piece_scale, 12, 12 );
        
        
        var lookTo = new THREE.Vector3();
        var referenceUp = new THREE.Vector3(0,0,1);

        var nodeInfo = this.Game.Board.Board.Blocks;
        this.GameBlocks = [];
        for (var ni=0; ni<nodeInfo.length; ni++) {
            var blockDecl = nodeInfo[ni];
            var state = this.Game.State.Core.Blocks[ni];
            var blockObj = this.BuildBlock( blockDecl, state );

            this.GameBlocks.push( blockObj );
            this.SceneRoot.add( blockObj );
        }
        this.GameGoal = this.BuildBlock( this.Game.Board.Board.Goal );
        this.SceneRoot.add( this.GameGoal );

        this.SceneRoot.add( this.BuildBackground( this.Game.Board.Board.Bounds ) );


        if (true) {
            this.Cursors = [ null ];
            for (var ti=0; ti<1; ti++) {
                const cursorObj = new THREE.Mesh( geometry, this.MatsByType[ti] );
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
