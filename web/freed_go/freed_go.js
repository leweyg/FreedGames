
var FreedGoGame_Create = function() {
    var ans = Object.create( FreedGoPrototype_Game );
    ans.Setup();
    return ans;
};

var FreedGoPrototype_State = {
    Game : null,
    Core : {
        Turn : 0,
        Nodes : [],
        Taken : [],
        Hovers : [],
    },
    OnCoreUpdated : (() => {}),

    Setup : function(game) {
        this.Game = game;
        this.NewBoard();
    },

    DoCoreChangedLocally : function() {
        this.OnCoreUpdated();
    },

    NewBoard : function() {
        var board = this.Game.Board;
        var numNodes = board.Nodes.length;
        this.Core.Turn = 0;
        this.Core.Nodes = []; 
        this.Core.Nodes.length = numNodes;
        this.Core.Taken = [ 0, 0 ];
        this.Core.Hovers = [ -1, -1 ];
        for (var i=0; i<numNodes; i++) {
            this.Core.Nodes[i] = -1;
        }
    },
};

var FreedGoThreeJS_Prototype = {
    Game : null,
    SceneParent : null,
    Stones : [],
    Cursors : [],
    MatsByType : {},
    RedrawCallback : null,
    DefaultScale : new THREE.Vector3(1,1,0.3),
    Temp1 : new THREE.Vector3(),

    Setup : function(game) {
        this.Game = game;
    },

    Update : function() {

        for (var ni=0; ni<this.Stones.length; ni++) {
            var state = this.Game.State.Core.Nodes[ni];
            var from = this.Stones[ni];
            var mat = this.MatsByType[ state ];
            var scl = this.Temp1;
            if (from.material != mat) {
                from.material = mat;
            }
            scl.copy( this.DefaultScale );
            if (state < 0) {
                scl.multiplyScalar( 0.61 );
                scl.z = this.DefaultScale.z;
            }
            if (from.scale.x != scl.x) {
                from.scale.copy( scl );
            }
        }

        for (var ti=0; ti<2; ti++) {
            var cursor = this.Cursors[ti];
            var hover = this.Game.State.Core.Hovers[ti];
            if (hover >= 0) {
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

    Build : function( scene ) {
        this.SceneParent = scene;
        this.Stones = [];

        this.MatsByType = {};
        this.MatsByType[-1] = new THREE.MeshLambertMaterial( { color: 0xb17c52 } );
        this.MatsByType[ 0] = new THREE.MeshLambertMaterial( { color: 0xffFFff } );
        this.MatsByType[ 1] = new THREE.MeshLambertMaterial( { color: 0x656565 } );

        const piece_scale = 26 * this.Game.Board.NodeScale;

        const geometry = new THREE.SphereBufferGeometry( piece_scale, 12, 12 );
        const board_scale = 35.0;
        
        var lookTo = new THREE.Vector3();
        var referenceUp = new THREE.Vector3(0,0,1);

        var nodeInfo = this.Game.Board.Nodes;
        for (var ni=0; ni<nodeInfo.length; ni++) {
            var node = nodeInfo[ni];

            const stone = new THREE.Mesh( geometry, this.MatsByType[-1] );
            stone.position.copy( node.Position );
            stone.position.multiplyScalar( board_scale );
            stone.userData.Node = node;
            this.Stones.push( stone );

            lookTo.copy( node.Normal );
            //lookTo.cross( referenceUp );
            lookTo.add( node.Position );
            lookTo.multiplyScalar( board_scale );
            stone.lookAt( lookTo );

            stone.scale.copy( this.DefaultScale );

            //stone.position.copy( node.Position );

            this.SceneParent.add( stone );
        }

        if (true) {
            var linePoints = [];
            for (var ni=0; ni<nodeInfo.length; ni++) {
                var node = nodeInfo[ni];
                var from = this.Stones[ni];
                for (var ei=0; ei<node.Neighbors.length; ei++) {
                    var to = this.Stones[ node.Neighbors[ei] ];
                    linePoints.push( from.position.clone() );
                    linePoints.push( to.position.clone() );
                }
            }
            const lineMat = new THREE.LineBasicMaterial({
                color: 0x000000
            });
            const lineGeo = new THREE.BufferGeometry().setFromPoints( linePoints );
            const lineObj = new THREE.LineSegments( lineGeo, lineMat );
            this.SceneParent.add( lineObj );
        }

        if (true) {
            this.Cursors = [ null, null ];
            for (var ti=0; ti<2; ti++) {
                const cursorObj = new THREE.Mesh( geometry, this.MatsByType[ti] );
                cursorObj.scale.set( 0.5, 0.5, 0.5 );
                this.SceneParent.add( cursorObj );
                this.Cursors[ti] = cursorObj;
            }
        }

        this.Update();
    }
};

var FreedGoPrototype_Game = {
    Board : FreedGo_Boards_Mobius,
    State : FreedGoPrototype_State,
    Render : FreedGoThreeJS_Prototype,

    Setup : function() {
        this.Board = FreedGo_Boards_Mobius;
        this.State = Object.create( FreedGoPrototype_State );
        this.State.Setup( this );
        this.Render = Object.create( FreedGoThreeJS_Prototype );
        this.Render.Setup( this );

        this.State.OnCoreUpdated = (() => {
            this.Render.Update();
        });
    },

    OnHoveredOver : function(index) {
        this.State.Core.Hovers[ this.State.Core.Turn ] = index;
        this.State.DoCoreChangedLocally();
    },

    OnClickedIndex : function(index) {
        this.State.Core.Nodes[ index ] = this.State.Core.Turn;
        this.State.Core.Turn = ((this.State.Core.Turn + 1)%2);
        this.State.DoCoreChangedLocally();
    },

};
