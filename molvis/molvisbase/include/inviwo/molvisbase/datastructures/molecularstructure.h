/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/
#pragma once

#include <inviwo/molvisbase/molvisbasemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/datatraits.h>
#include <inviwo/core/util/document.h>

#include <optional>

namespace inviwo {

namespace molvis {

// (structure id, model id, chain id, residue id, atom name, altloc).

struct IVW_MODULE_MOLVISBASE_API Atoms {
    void updateAtomNumbers();
    bool empty() const;
    size_t size() const;
    void clear();

    std::vector<dvec3> positions;
    std::vector<double> bfactors;
    // std::vector<int> structureIds;
    std::vector<int> modelIds;
    std::vector<int> chainIds;
    std::vector<int> residueIds;
    std::vector<unsigned char> atomNumbers;
    std::vector<std::string> fullNames;

    std::vector<int> residueIndices;
};

struct IVW_MODULE_MOLVISBASE_API Residue {
    size_t id;
    std::string name;
    std::string fullName;
    size_t chainId;
    std::set<size_t> atoms;
};

struct IVW_MODULE_MOLVISBASE_API Chain {
    size_t id;
    std::string name;
    std::set<size_t> residues;
};

using Bond = std::pair<size_t, size_t>;

class IVW_MODULE_MOLVISBASE_API MolecularStructure {
public:
    MolecularStructure();
    virtual ~MolecularStructure() = default;

    void setSource(const std::string& source);
    const std::string& getSource() const;

    size_t getAtomCount() const;
    size_t getResidueCount() const;
    size_t getChainCount() const;
    size_t getBondCount() const;

    void setAtoms(Atoms atoms);
    void setResidues(std::vector<Residue> residues);
    void setChains(std::vector<Chain> chains);

    const Atoms& getAtoms() const;
    const std::vector<Residue>& getResidues() const;
    const std::vector<Chain>& getChains() const;

    std::optional<Residue> getResidue(size_t residueId, size_t chainId) const;
    std::optional<Chain> getChain(size_t chainId) const;

    std::optional<size_t> getAtomIndex(const std::string& fullAtomName, size_t residueId,
                                       size_t chainId) const;

    /**
     * \brief update internal state of residues and chains, i.e. contained atoms/residues
     */
    void updateStructure();

    void computeCovalentBonds();
    const std::vector<Bond>& getBonds() const;

private:
    std::string source_;

    Atoms atoms_;
    std::vector<Residue> residues_;
    std::vector<Chain> chains_;
    std::vector<Bond> bonds_;
};

}  // namespace molvis

template <>
struct DataTraits<molvis::MolecularStructure> {
    static std::string classIdentifier() { return "org.inviwo.molvis.MolecularStructure"; }
    static std::string dataName() { return "Molecular Structure"; }
    static uvec3 colorCode() { return uvec3(56, 127, 66); }
    static Document info(const molvis::MolecularStructure& data) {
        using H = utildoc::TableBuilder::Header;
        using P = Document::PathComponent;
        Document doc;
        doc.append("b", "Molecular Structure", {{"style", "color:white;"}});
        utildoc::TableBuilder tb(doc.handle(), P::end());
        tb(H("Source"), data.getSource());
        tb(H("Atoms"), data.getAtomCount());
        tb(H("Residues"), data.getResidueCount());
        tb(H("Chains"), data.getChainCount());
        tb(H("Bonds"), data.getBondCount());

        return doc;
    }
};

}  // namespace inviwo
