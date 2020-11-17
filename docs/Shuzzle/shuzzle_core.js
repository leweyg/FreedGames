
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
    }
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
        this.Fast.LightPos = TensorMath.cloneVector3( this.Game.Board.Board.Bounds.max );
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
        console.log("clicked " + grabData.type + " " + index);
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

