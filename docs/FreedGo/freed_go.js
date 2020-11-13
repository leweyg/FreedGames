
var FreedGoGame_Create = function() {
    var ans = Object.create( FreedGoPrototype_Game );
    ans.Setup();
    return ans;
};

var FreedGoSaveCallbacks = {
    DoSaveCore : ((core) => {
        //console.log("TODO: Save core");
    }),
    DoSaveFast : ((fast) => {
        //console.log("TODO: Save core");
    }),
};

var FreedGoPrototype_State = {
    Game : null,
    Fast : {
        Hovers : [],
    },
    Core : {
        BoardName : "mobius",
        Turn : 0,
        Nodes : [],
        Taken : [],
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
        var board = this.Game.Board;
        var numNodes = board.Nodes.length;
        this.Core.BoardName = this.Game.Board.Name;
        this.Core.Turn = 0;
        this.Core.Nodes = []; 
        this.Core.Nodes.length = numNodes;
        this.Core.Taken = [ 0, 0 ];
        this.Fast.Hovers = [ -1, -1 ];
        for (var i=0; i<numNodes; i++) {
            this.Core.Nodes[i] = -1;
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
        FreedGoSaveCallbacks.DoSaveFast( this.Fast );
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
        FreedGoSaveCallbacks.DoSaveCore( this.Core );
    },

    CalcGroup : function(index) {
        var result = {
            Frees : 0,
            Size : 0,
            Closeds : 0,
            GroupType : 0,
            Indices : {},
            Others : {},
            Queue : [],
        };
        result.Queue.push( index );
        result.GroupType = this.Core.Nodes[ index ];;
        while (result.Queue.length > 0) {
            var cur = result.Queue.pop();
            var curType = this.Core.Nodes[ cur ];
            if (curType == result.GroupType) {
                if (cur in result.Indices) continue;

                result.Size++;
                result.Indices[cur] = 1;
                var node = this.Game.Board.Nodes[cur];
                for (var ni in node.Neighbors) {
                    var oi = node.Neighbors[ni];
                    result.Queue.push(oi);
                }
            } else {
                if (cur in result.Others) continue;
                
                result.Others[cur] = 1;
                if (curType < 0) {
                    result.Frees++;
                } else {
                    result.Closeds++;
                }
            }
        }

        return result;
    },

    DoPlaceStone : function(index,to) {
        var was = this.Core.Nodes[ index ];
        this.Core.Nodes[ index ] = to;

        var node = this.Game.Board.Nodes[ index ];
        for (var ni in node.Neighbors) {
            var oi = node.Neighbors[ni];
            if (this.Core.Nodes[oi] < 0) continue;

            var group = this.CalcGroup(oi);
            if (group.Frees == 0) {
                // take the group:
                for (var di in group.Indices) {
                    this.Core.Nodes[di] = -1;
                    this.Core.Taken[to]++;
                }
            }
        }

        var atGroup = this.CalcGroup( index );
        if (atGroup.Frees == 0) {
            this.Core.Nodes[ index ] = was;
            // NOTE: revert all changes here, but probably weren't any
            return false;
        }

        return true;
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

var FreedGoThreeJS_Prototype = {
    Game : null,
    SceneRoot : null,
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

        if (!this.SceneRoot) return;

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

    Build : function( contextObj ) {

        if (this.SceneParent && this.SceneRoot) {
            this.SceneParent.remove( this.SceneRoot );
            this.SceneRoot = null;
        }

        this.SceneParent = contextObj;
        var scene = new THREE.Group();
        //scene.quaternion.setFromAxisAngle(new THREE.Vector3(0,0,1),-90);
        this.SceneRoot = scene;

        this.SceneParent.add( scene );
        
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

            this.SceneRoot.add( stone );
        }

        if (true) {
            var linePoints = [];
            for (var ni=0; ni<nodeInfo.length; ni++) {
                var node = nodeInfo[ni];
                var from = this.Stones[ni];
                for (var ei=0; ei<node.Neighbors.length; ei++) {
                    var ti = node.Neighbors[ei];
                    if (ti < ni) continue;
                    var to = this.Stones[ ti ];
                    linePoints.push( from.position.clone() );
                    linePoints.push( to.position.clone() );
                }
            }
            const lineMat = new THREE.LineBasicMaterial({
                color: 0x000000,
                //linewidth: 3,
            });
            const lineGeo = new THREE.BufferGeometry().setFromPoints( linePoints );
            const lineObj = new THREE.LineSegments( lineGeo, lineMat );
            this.SceneRoot.add( lineObj );
        }

        if (true) {
            this.Cursors = [ null, null ];
            for (var ti=0; ti<2; ti++) {
                const cursorObj = new THREE.Mesh( geometry, this.MatsByType[ti] );
                cursorObj.scale.set( 0.5, 0.5, 0.5 );
                this.SceneRoot.add( cursorObj );
                this.Cursors[ti] = cursorObj;
            }
        }

        this.Update();
    }
};

var FreedGo_Boards_Mobius = FreedGoBoards["mobius"];

var FreedGoPrototype_Game = {
    Board : FreedGo_Boards_Mobius,
    State : FreedGoPrototype_State,
    Render : FreedGoThreeJS_Prototype,
    OnCoreUpdated : [],
    OnFastUpdated : [],

    CheckBoardName : function(name) {
        if (this.Board.Name == name) return;
        var to = FreedGoBoards[name];
        if (!to) {
            console.log("Unknown board '" + name + "'");
            return;
        }

        this.ChangeBoard(to);
    },

    ChangeBoard : function( board ) {
        this.Board = board || FreedGo_Boards_Mobius;
        var state = Object.create( FreedGoPrototype_State );
        this.State = state;
        this.State.Setup( this );
        if (!this.Render) {
            this.Render = Object.create( FreedGoThreeJS_Prototype );
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
