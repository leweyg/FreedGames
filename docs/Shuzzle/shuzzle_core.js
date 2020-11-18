
var ShuzzleSaveCallbacks = {
    DoSaveCore : ((core) => {
        //console.log("TODO: Save core");
    }),
    DoSaveFast : ((fast) => {
        //console.log("TODO: Save core");
    }),
};

var TensorMath = {
    cloneVector3 : function(v) {
        return { x:v.x, y:v.y, z:v.z };
    },
    copyVector3Into : function(dst, src) {
        dst.x = src.x;
        dst.y = src.y;
        dst.z = src.z;
        return dst;
    },
    cloneBounds : function(b) {
        return {
            min:TensorMath.cloneVector3(b.min),
            max:TensorMath.cloneVector3(b.max),
        };
    },
    clampVector3InBounds : function(v,b) {
        v.x = Math.max( b.min.x, Math.min( b.max.x, v.x ) );
        v.y = Math.max( b.min.y, Math.min( b.max.y, v.y ) );
        v.z = Math.max( b.min.z, Math.min( b.max.z, v.z ) );
    },
    vector3MaxAxisDistanceXY : function(a,b) {
        return Math.min( Math.abs(a.x-b.x), Math.abs(a.y-b.y));
    },
    _tempVector3 : new THREE.Vector3(),
    boundsMaxAxisDistanceXY : function(b,v) {
        var t = TensorMath._tempVector3;
        t.copy( b.min );
        var s = TensorMath.vector3MaxAxisDistanceXY( t, v );

        t.copy( b.min );
        t.y = b.max.y;
        s = Math.min( s, TensorMath.vector3MaxAxisDistanceXY(t, v) );

        t.copy( b.min );
        t.x = b.max.x;
        s = Math.min( s, TensorMath.vector3MaxAxisDistanceXY(t, v) );

        t.copy( b.max );
        s = Math.min( s, TensorMath.vector3MaxAxisDistanceXY(t, v) );

        return s;
    },
};

var ShuzzlePrototype_State = {
    Game : null,
    Fast : {
        Hovers : [],
        LightPos : {x:0,y:0,z:0},
    },
    Core : {
        BoardName : "1",
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
        var bounds = this.Game.Board.Board.Bounds;
        this.Fast.LightPos = TensorMath.cloneVector3( bounds.max );
        this.Fast.LightPos.x = ( bounds.min.x + bounds.max.x ) * 0.5;
        //this.Fast.LightPos.x = this.Game.Board.Board.Bounds.min.x;
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

    DoClickedIndex : function(grabData) {
        var index = grabData.index;
        //console.log("clicked " + grabData.type + " " + index);
        if (index==undefined) return;
        if ((index < 0) || (index >= this.Core.Blocks.length)) {
            return false;
        }
        var cur = this.Core.Blocks[ index ];
        /*
        var to = this.Core.Turn;
        if (cur >= 0) {
            to = -1;
            var otherTurn = ((cur + 1)%2);
            this.Core.Taken[otherTurn]++;
        }
        if (this.DoPlaceStone( index, to )) {
            this.Core.Turn = ((this.Core.Turn + 1)%2);
        }
        */
        this.DoCoreChangedLocally();
    }

};

