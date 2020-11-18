
var ShuzzleSaveCallbacks = {
    DoSaveCore : ((core) => {
        //console.log("TODO: Save core");
    }),
    DoSaveFast : ((fast) => {
        //console.log("TODO: Save core");
    }),
};

var numOr0 = function(v) {
    if (v) return v;
    return 0;
}

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
    isVector3InBounds : function(v,b) {
        var x = ((v.x >= b.min.x)&&(v.x <= b.max.x));
        var y = ((v.y >= b.min.y)&&(v.y <= b.max.y));
        var z = ((v.z >= b.min.z)&&(v.z <= b.max.z));
        return (x && y && z);
    },
    clampVector3InBounds : function(v,b) {
        v.x = Math.max( b.min.x, Math.min( b.max.x, v.x ) );
        v.y = Math.max( b.min.y, Math.min( b.max.y, v.y ) );
        v.z = Math.max( b.min.z, Math.min( b.max.z, v.z ) );
    },
    vector3MaxAxisDistanceXY : function(a,b) {
        return Math.min( Math.abs(a.x-b.x), Math.abs(a.y-b.y));
    },
    vector3ApplyTransform : function(v,t,into) {
        into.x = (numOr0(t.x.x) * v.x) + (numOr0(t.x.y) * v.y) + (numOr0(t.x.z) * v.z);
        into.y = (numOr0(t.y.x) * v.x) + (numOr0(t.y.y) * v.y) + (numOr0(t.y.z) * v.z);
        into.z = (numOr0(t.z.x) * v.x) + (numOr0(t.z.y) * v.y) + (numOr0(t.z.z) * v.z);
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

var ShuzzlePrototype_Voxels = {
    Size : new THREE.Vector3(),
    Bounds : null,
    VoxelData : [],
    TotalVoxels : 0,
    _TempForIndexing : new THREE.Vector3(),

    SetupFromBounds : function(_bounds) {
        this.Bounds = _bounds;
        this.Size.copy( this.Bounds.max );
        this.Size.sub( this.Bounds.min );

        this.TotalVoxels = this.Size.x * this.Size.y * this.Size.z;
        this.VoxelData = []; this.VoxelData.length = this.TotalVoxels;
        for (var vi=0; vi<this.TotalVoxels; vi++) {
            this.VoxelData[vi] = ' ';
        }
    },

    SafeGetIndex : function(v) {
        if (TensorMath.isVector3InBounds(v, this.Bounds)) {
            var i = ( ((v.x - this.Bounds.min.x)*1)
                + ((v.y - this.Bounds.min.y)*this.Size.x)
                + ((v.z - this.Bounds.min.z)*(this.Size.x * this.Size.y)) );
            if ((i <= 0) && (i >= this.TotalVoxels)) {
                alert("Indexing error: " + i + " for " + v);
            }
            return i;
        }
        return -1;
    },
    
    TryGetBoxVoxelIndex : function (blockState, boxLocalPos ) {
        var t= this._TempForIndexing;
        TensorMath.vector3ApplyTransform( boxLocalPos, blockState.RotationTrans, t );
        t.add( blockState.Center );
        var ndx = this.SafeGetIndex( t );
        return ndx;
    },

    DrawBlock : function(blockState, blockDef, letter) {
        var nBoxes = blockDef.Boxes.length;
        if (letter === undefined) letter = blockDef.Letter;
        for (var bi=0; bi<nBoxes; bi++) {
            var boxLocalPos = blockDef.Boxes[bi];
            var ndx = this.TryGetBoxVoxelIndex( blockState, boxLocalPos );
            if (ndx >= 0) {
                var was = this.VoxelData[ndx];
                if ((was==" ")||(letter==" ")) {
                    this.VoxelData[ndx] = letter;
                } else {
                    throw "Invalid voxel write over '" + was + "'";
                }
            }
        }
    },

    ClearAll : function() {
        for (var vi=0; vi<this.TotalVoxels; vi++) {
            this.VoxelData[vi] = ' ';
        }
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
    Voxels : ShuzzlePrototype_Voxels,

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

        this.Voxels = Object.create( ShuzzlePrototype_Voxels );
        this.Voxels.SetupFromBounds( bounds );

        //this.Fast.LightPos.x = this.Game.Board.Board.Bounds.min.x;
        for (var i=0; i<numNodes; i++) {
            board.Blocks[i].Letter = String.fromCharCode( board.Blocks[i].Index + "a".charCodeAt(0) );
            this.Core.Blocks[i] = { 
                Center : {x:0, y:0, z:0},
                RotationTrans : {
                    x:{x:1}, y:{y:1}, z:{z:1}
                }
            };
            TensorMath.copyVector3Into( this.Core.Blocks[i].Center, board.Blocks[i].Center );
            this.Voxels.DrawBlock( this.Core.Blocks[i], board.Blocks[i] );
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
            //this.Game.Board 
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

