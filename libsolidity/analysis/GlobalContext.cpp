/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-3.0
/**
 * @author Christian <c@ethdev.com>
 * @author Gav Wood <g@ethdev.com>
 * @date 2014
 * Container of the (implicit and explicit) global objects.
 */

#include <libsolidity/analysis/GlobalContext.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/TypeProvider.h>
#include <libsolidity/ast/Types.h>
#include <memory>

namespace solidity::frontend
{

namespace
{
/// Magic variables get negative ids for easy differentiation
int magicVariableToID(std::string const& _name)
{
	if (_name == "abi") return -1;
	else if (_name == "addmod") return -2;
	else if (_name == "assert") return -3;
	else if (_name == "block") return -4;
	else if (_name == "blockhash") return -5;
	else if (_name == "ecrecover") return -6;
	else if (_name == "gasleft") return -7;
	else if (_name == "keccak256") return -8;
	else if (_name == "msg") return -15;
	else if (_name == "mulmod") return -16;
	else if (_name == "now") return -17;
	else if (_name == "require") return -18;
	else if (_name == "revert") return -19;
	else if (_name == "ripemd160") return -20;
	else if (_name == "selfdestruct") return -21;
	else if (_name == "sha256") return -22;
	else if (_name == "sha3") return -23;
	else if (_name == "suicide") return -24;
	else if (_name == "super") return -25;
	else if (_name == "tx") return -26;
	else if (_name == "type") return -27;
	else if (_name == "this") return -28;
	else if (_name == "verifyMintProof") return -29;
	else if (_name == "verifyBurnProof") return -30;
	else if (_name == "verifyTransferProof") return -31;
	else if (_name == "pedersenHash") return -32;
	else if (_name == "batchvalidatesign") return -33;
	else if (_name == "validatemultisign") return -34;
	else if (_name == "freeze") return -35;
	else if (_name == "unfreeze") return -36;
	else if (_name == "freezeExpireTime") return -37;
	else if (_name == "withdrawreward") return -38;
	else if (_name == "vote") return -39;
	else if (_name == "rewardBalance") return -40;
	else if (_name == "isSrCandidate") return -41;
	else if (_name == "voteCount") return -42;
	else if (_name == "totalVoteCount") return -43;
	else if (_name == "receivedVoteCount") return -44;
	else if (_name == "usedVoteCount") return -45;
	else if (_name == "freezebalancev2") return -46;
	else if (_name == "unfreezebalancev2") return -47;
	else if (_name == "cancelallunfreezev2") return -48;
	else if (_name == "withdrawexpireunfreeze") return -49;
	else if (_name == "chain") return -50;
	else if (_name == "getchainparameter") return -51;
	else if (_name == "blobhash") return -52;
	else
		solAssert(false, "Unknown magic variable: \"" + _name + "\".");
}

inline std::vector<std::shared_ptr<MagicVariableDeclaration const>> constructMagicVariables(langutil::EVMVersion _evmVersion)
{
	static auto const magicVarDecl = [](std::string const& _name, Type const* _type) {
		return std::make_shared<MagicVariableDeclaration>(magicVariableToID(_name), _name, _type);
	};

	std::vector<std::shared_ptr<MagicVariableDeclaration const>> magicVariableDeclarations = {
		magicVarDecl("abi", TypeProvider::magic(MagicType::Kind::ABI)),
		magicVarDecl("addmod", TypeProvider::function(strings{"uint256", "uint256", "uint256"}, strings{"uint256"}, FunctionType::Kind::AddMod, StateMutability::Pure)),
		magicVarDecl("assert", TypeProvider::function(strings{"bool"}, strings{}, FunctionType::Kind::Assert, StateMutability::Pure)),
		magicVarDecl("block", TypeProvider::magic(MagicType::Kind::Block)),
		magicVarDecl("blockhash", TypeProvider::function(strings{"uint256"}, strings{"bytes32"}, FunctionType::Kind::BlockHash, StateMutability::View)),
		magicVarDecl("ecrecover", TypeProvider::function(strings{"bytes32", "uint8", "bytes32", "bytes32"}, strings{"address"}, FunctionType::Kind::ECRecover, StateMutability::Pure)),
		magicVarDecl("gasleft", TypeProvider::function(strings(), strings{"uint256"}, FunctionType::Kind::GasLeft, StateMutability::View)),
		magicVarDecl("keccak256", TypeProvider::function(strings{"bytes memory"}, strings{"bytes32"}, FunctionType::Kind::KECCAK256, StateMutability::Pure)),
		magicVarDecl("msg", TypeProvider::magic(MagicType::Kind::Message)),
		magicVarDecl("mulmod", TypeProvider::function(strings{"uint256", "uint256", "uint256"}, strings{"uint256"}, FunctionType::Kind::MulMod, StateMutability::Pure)),
		magicVarDecl("now", TypeProvider::uint256()),
		magicVarDecl("require", TypeProvider::function(strings{"bool"}, strings{}, FunctionType::Kind::Require, StateMutability::Pure)),
		magicVarDecl("require", TypeProvider::function(strings{"bool", "string memory"}, strings{}, FunctionType::Kind::Require, StateMutability::Pure)),
		magicVarDecl("revert", TypeProvider::function(strings(), strings(), FunctionType::Kind::Revert, StateMutability::Pure)),
		magicVarDecl("revert", TypeProvider::function(strings{"string memory"}, strings(), FunctionType::Kind::Revert, StateMutability::Pure)),
		magicVarDecl("ripemd160", TypeProvider::function(strings{"bytes memory"}, strings{"bytes20"}, FunctionType::Kind::RIPEMD160, StateMutability::Pure)),
		magicVarDecl("selfdestruct", TypeProvider::function(strings{"address payable"}, strings{}, FunctionType::Kind::Selfdestruct)),
		magicVarDecl("sha256", TypeProvider::function(strings{"bytes memory"}, strings{"bytes32"}, FunctionType::Kind::SHA256, StateMutability::Pure)),
		magicVarDecl("sha3", TypeProvider::function(strings{"bytes memory"}, strings{"bytes32"}, FunctionType::Kind::KECCAK256, StateMutability::Pure)),
		magicVarDecl("suicide", TypeProvider::function(strings{"address payable"}, strings{}, FunctionType::Kind::Selfdestruct)),
		magicVarDecl("tx", TypeProvider::magic(MagicType::Kind::Transaction)),
		magicVarDecl("freeze", TypeProvider::function(strings{"uint", "uint"}, strings{"bool"}, FunctionType::Kind::Freeze, StateMutability::NonPayable)),
		magicVarDecl("unfreeze", TypeProvider::function(strings{"uint"}, strings{"bool"}, FunctionType::Kind::Unfreeze, StateMutability::NonPayable)),
		magicVarDecl("freezeExpireTime",TypeProvider::function(strings{"uint"}, strings{"uint"}, FunctionType::Kind::FreezeExpireTime, StateMutability::NonPayable)),
		magicVarDecl("withdrawreward", TypeProvider::function(strings{}, strings{"uint"}, FunctionType::Kind::WithdrawReward)),
		magicVarDecl("freezebalancev2", TypeProvider::function(strings{"uint", "uint"}, strings(), FunctionType::Kind::FreezeBalanceV2, StateMutability::NonPayable)),
		magicVarDecl("unfreezebalancev2", TypeProvider::function(strings{"uint", "uint"}, strings(), FunctionType::Kind::UnfreezeBalanceV2, StateMutability::NonPayable)),
		magicVarDecl("cancelallunfreezev2", TypeProvider::function(strings(), strings{"uint"}, FunctionType::Kind::CancelAllUnfreezeV2, StateMutability::NonPayable)),
		magicVarDecl("withdrawexpireunfreeze", TypeProvider::function(strings{}, strings{"uint"}, FunctionType::Kind::WithdrawExpireUnfreeze, StateMutability::NonPayable)),
		magicVarDecl("chain", TypeProvider::magic(MagicType::Kind::Chain)),
		magicVarDecl("getchainparameter", TypeProvider::function(strings{"uint"}, strings{"uint"}, FunctionType::Kind::GetChainParameter, StateMutability::View)),
		// Accepts a MagicType that can be any contract type or an Integer type and returns a
		// MagicType. The TypeChecker handles the correctness of the input and output types.
		magicVarDecl("type", TypeProvider::function(
			strings{},
			strings{},
			FunctionType::Kind::MetaType,
			StateMutability::Pure,
			FunctionType::Options::withArbitraryParameters()
		)),
	};

	if (_evmVersion >= langutil::EVMVersion::cancun())
		magicVariableDeclarations.push_back(
			magicVarDecl("blobhash", TypeProvider::function(strings{"uint256"}, strings{"bytes32"}, FunctionType::Kind::BlobHash, StateMutability::View))
		);

	return magicVariableDeclarations;
}

}

GlobalContext::GlobalContext(langutil::EVMVersion _evmVersion):
	m_magicVariables{constructMagicVariables(_evmVersion)}
{
	addBatchValidateSignMethod();
	addValidateMultiSignMethod();
	addVerifyMintProofMethod();
	addVerifyBurnProofMethod();
	addVerifyTransferProofMethod();
	addPedersenHashMethod();
	addVoteMethod();
	addRewardBalanceMethod();
	addIsSRCandidateMethod();
	addVoteCountMethod();
	addTotalVoteCountMethod();
	addReceivedVoteCountMethod();
	addUsedVoteCountMethod();
}

void GlobalContext::addVerifyMintProofMethod() {
	TypePointers parameterTypes;
	//output bytes32[9]
	parameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::fixedBytes(32),u256(9)));
	//bindingSignature bytes32[2]
	parameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::fixedBytes(32),u256(2)));
	//value uint64
	parameterTypes.push_back(TypeProvider::uint(64));
	//signHash bytes32
	parameterTypes.push_back(TypeProvider::fixedBytes(32));
	//frontier bytes32[33]
	parameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::fixedBytes(32),u256(33)));
	//leafCount uint256
	parameterTypes.push_back(TypeProvider::uint256());

	TypePointers returnParameterTypes;
	returnParameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::fixedBytes(32)));
	strings parameterNames;
	parameterNames.push_back("output");
	parameterNames.push_back("bindingSignature");
	parameterNames.push_back("value");
	parameterNames.push_back("signHash");
	parameterNames.push_back("frontier");
	parameterNames.push_back("leafCount");

	strings returnParameterNames;
	returnParameterNames.push_back("msg");

	m_magicVariables.push_back(std::make_shared<MagicVariableDeclaration>(magicVariableToID("verifyMintProof"), "verifyMintProof", TypeProvider::function(
			parameterTypes,
			returnParameterTypes,
			parameterNames,
			returnParameterNames,
			FunctionType::Kind::VerifyMintProof,
			StateMutability::Pure,
			nullptr)
	));
}


void GlobalContext::addVerifyBurnProofMethod() {
	TypePointers parameterTypes;
	//input bytes32[10]
	parameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::fixedBytes(32), u256(10)));
	//spend_auth_sig bytes32[2]
	parameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::fixedBytes(32), u256(2)));
	//value uint64
	parameterTypes.push_back(TypeProvider::uint(64));
	//bindingSignature  bytes32[2]
	parameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::fixedBytes(32), u256(2)));
	//signHash bytes32
	parameterTypes.push_back(TypeProvider::fixedBytes(32));

	TypePointers returnParameterTypes;
	returnParameterTypes.push_back(TypeProvider::boolean());
	strings parameterNames;
	parameterNames.push_back("input");
	parameterNames.push_back("spend_auth_sig");
	parameterNames.push_back("value");
	parameterNames.push_back("bindingSignature");
	parameterNames.push_back("signHash");


	strings returnParameterNames;
	returnParameterNames.push_back("msg");

	m_magicVariables.push_back(std::make_shared<MagicVariableDeclaration>(magicVariableToID("verifyBurnProof"), "verifyBurnProof", TypeProvider::function(
			parameterTypes,
			returnParameterTypes,
			parameterNames,
			returnParameterNames,
			FunctionType::Kind::VerifyBurnProof,
			StateMutability::Pure,
			nullptr)));

}


void GlobalContext::addVerifyTransferProofMethod() {
	TypePointers parameterTypes;
	//bytes32[10][] input
	parameterTypes.push_back(TypeProvider::array(DataLocation::Memory,
												 TypeProvider::array(DataLocation::Memory,TypeProvider::fixedBytes(32),u256(10))
	));
	//spend_auth_sig bytes32[2][]
	parameterTypes.push_back(TypeProvider::array(DataLocation::Memory,
												 TypeProvider::array(DataLocation::Memory,TypeProvider::fixedBytes(32),u256(2))
	));
	//output bytes32[9][]
	parameterTypes.push_back(TypeProvider::array(DataLocation::Memory,
												 TypeProvider::array(DataLocation::Memory,TypeProvider::fixedBytes(32),u256(9))
	));
	//bindingSignature bytes32[2]
	parameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::fixedBytes(32),u256(2)));
	//signHash bytes32
	parameterTypes.push_back(TypeProvider::fixedBytes(32));
	//value uint256
	parameterTypes.push_back(TypeProvider::uint(64));
	//frontier bytes32[33]
	parameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::fixedBytes(32),u256(33)));
	//leafCount uint256
	parameterTypes.push_back(TypeProvider::uint256());

	TypePointers returnParameterTypes;
	returnParameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::fixedBytes(32)));
	strings parameterNames;
	parameterNames.push_back("input");
	parameterNames.push_back("spend_auth_sig");
	parameterNames.push_back("output");
	parameterNames.push_back("bindingSignature");
	parameterNames.push_back("signHash");
	parameterNames.push_back("value");
	parameterNames.push_back("frontier");
	parameterNames.push_back("leafCount");

	strings returnParameterNames;
	returnParameterNames.push_back("msg");

	m_magicVariables.push_back(std::make_shared<MagicVariableDeclaration>(magicVariableToID("verifyTransferProof"), "verifyTransferProof", TypeProvider::function(
			parameterTypes,
			returnParameterTypes,
			parameterNames,
			returnParameterNames,
			FunctionType::Kind::VerifyTransferProof,
			StateMutability::Pure,
			nullptr)
	));
}

void GlobalContext::addPedersenHashMethod() {
	TypePointers parameterTypes;
	//i uint32
	parameterTypes.push_back(TypeProvider::uint(32));
	//left bytes32
	parameterTypes.push_back(TypeProvider::fixedBytes(32));
	//right bytes32
	parameterTypes.push_back(TypeProvider::fixedBytes(32));


	TypePointers returnParameterTypes;
	returnParameterTypes.push_back(TypeProvider::fixedBytes(32));
	strings parameterNames;
	parameterNames.push_back("i");
	parameterNames.push_back("left");
	parameterNames.push_back("right");


	strings returnParameterNames;
	returnParameterNames.push_back("msg");

	m_magicVariables.push_back(std::make_shared<MagicVariableDeclaration>(magicVariableToID("pedersenHash"), "pedersenHash", TypeProvider::function(
			parameterTypes,
			returnParameterTypes,
			parameterNames,
			returnParameterNames,
			FunctionType::Kind::PedersenHash,
			StateMutability::Pure,
			nullptr)
	));
}


void GlobalContext::addBatchValidateSignMethod() {
	// bool multivalidatesign(bytes32 hash, bytes[] memory signatures, address[] memory addresses)
	TypePointers parameterTypes;
	parameterTypes.push_back(TypeProvider::fixedBytes(32));
	parameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::bytesMemory()));
	parameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::address()));

	TypePointers returnParameterTypes;
	returnParameterTypes.push_back(TypeProvider::fixedBytes(32));
	strings parameterNames;
	parameterNames.push_back("hash");
	parameterNames.push_back("signatures");
	parameterNames.push_back("addresses");
	strings returnParameterNames;
	returnParameterNames.push_back("ok");

	m_magicVariables.push_back(std::make_shared<MagicVariableDeclaration>(magicVariableToID("batchvalidatesign"), "batchvalidatesign", TypeProvider::function(
			parameterTypes,
			returnParameterTypes,
			parameterNames,
			returnParameterNames,
			FunctionType::Kind::BatchValidateSign,
			StateMutability::Pure,
			nullptr)
	));
}


void GlobalContext::addValidateMultiSignMethod() {
	// bool multivalidatesign(bytes32 hash, bytes[] memory signatures, address[] memory addresses)
	TypePointers parameterTypes;
	parameterTypes.push_back(TypeProvider::address());
	parameterTypes.push_back(TypeProvider::uint256());
	parameterTypes.push_back(TypeProvider::fixedBytes(32));
	parameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::bytesMemory()));

	TypePointers returnParameterTypes;
	returnParameterTypes.push_back(TypeProvider::boolean());
	strings parameterNames;
	parameterNames.push_back("address");
	parameterNames.push_back("permissonid");
	parameterNames.push_back("content");
	parameterNames.push_back("signatures");
	strings returnParameterNames;
	returnParameterNames.push_back("ok");

	m_magicVariables.push_back(std::make_shared<MagicVariableDeclaration>(magicVariableToID("validatemultisign"), "validatemultisign", TypeProvider::function(
			parameterTypes,
			returnParameterTypes,
			parameterNames,
			returnParameterNames,
			FunctionType::Kind::ValidateMultiSign,
			StateMutability::Pure,
			nullptr)
	));
}

void GlobalContext::addVoteMethod() {
	// void vote(address[] memory addresses, unit256[] tronpowerlist)
	TypePointers parameterTypes;

	parameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::address()));
	parameterTypes.push_back(TypeProvider::array(DataLocation::Memory, TypeProvider::uint256()));

	TypePointers returnParameterTypes;
	strings parameterNames;
	parameterNames.push_back("srList");
	parameterNames.push_back("tronpowerList");
	strings returnParameterNames;

	m_magicVariables.push_back(std::make_shared<MagicVariableDeclaration>(magicVariableToID("vote"), "vote", TypeProvider::function(
		parameterTypes,
		returnParameterTypes,
		parameterNames,
		returnParameterNames,
		FunctionType::Kind::Vote,
		StateMutability::NonPayable,
		nullptr)
	));
}

void GlobalContext::addRewardBalanceMethod() {
	// uint rewardBalance()
	TypePointers parameterTypes;
	TypePointers returnParameterTypes;
	returnParameterTypes.push_back(TypeProvider::uint256());
	strings parameterNames;
	strings returnParameterNames;
	returnParameterNames.push_back("result");

	m_magicVariables.push_back(std::make_shared<MagicVariableDeclaration>(magicVariableToID("rewardBalance"), "rewardBalance", TypeProvider::function(
		parameterTypes,
		returnParameterTypes,
		parameterNames,
		returnParameterNames,
		FunctionType::Kind::RewardBalance,
		StateMutability::View,
		nullptr)
	));
}

void GlobalContext::addIsSRCandidateMethod() {
	// bool isSrCandidate(address)
	TypePointers parameterTypes;
	parameterTypes.push_back(TypeProvider::address());

	TypePointers returnParameterTypes;
	returnParameterTypes.push_back(TypeProvider::boolean());
	strings parameterNames;
	parameterNames.push_back("address");
	strings returnParameterNames;
	returnParameterNames.push_back("ok");

	m_magicVariables.push_back(std::make_shared<MagicVariableDeclaration>(magicVariableToID("isSrCandidate"), "isSrCandidate", TypeProvider::function(
		parameterTypes,
		returnParameterTypes,
		parameterNames,
		returnParameterNames,
		FunctionType::Kind::IsSrCandidate,
		StateMutability::View,
		nullptr)
	));
}

void GlobalContext::addVoteCountMethod() {
	// uint voteCount(address, address)
	TypePointers parameterTypes;
	parameterTypes.push_back(TypeProvider::address());
	parameterTypes.push_back(TypeProvider::address());

	TypePointers returnParameterTypes;
	returnParameterTypes.push_back(TypeProvider::uint256());
	strings parameterNames;
	parameterNames.push_back("address");
	parameterNames.push_back("address");
	strings returnParameterNames;
	returnParameterNames.push_back("result");

	m_magicVariables.push_back(std::make_shared<MagicVariableDeclaration>(magicVariableToID("voteCount"), "voteCount", TypeProvider::function(
		parameterTypes,
		returnParameterTypes,
		parameterNames,
		returnParameterNames,
		FunctionType::Kind::VoteCount,
		StateMutability::View,
		nullptr)
	));
}

void GlobalContext::addTotalVoteCountMethod() {
	// uint totalVoteCount(address)
	TypePointers parameterTypes;
	parameterTypes.push_back(TypeProvider::address());

	TypePointers returnParameterTypes;
	returnParameterTypes.push_back(TypeProvider::uint256());
	strings parameterNames;
	parameterNames.push_back("address");
	strings returnParameterNames;
	returnParameterNames.push_back("result");

	m_magicVariables.push_back(std::make_shared<MagicVariableDeclaration>(magicVariableToID("totalVoteCount"), "totalVoteCount", TypeProvider::function(
		parameterTypes,
		returnParameterTypes,
		parameterNames,
		returnParameterNames,
		FunctionType::Kind::TotalVoteCount,
		StateMutability::View,
		nullptr)
	));
}

void GlobalContext::addReceivedVoteCountMethod() {
	// uint receivedVoteCount(address)
	TypePointers parameterTypes;
	parameterTypes.push_back(TypeProvider::address());

	TypePointers returnParameterTypes;
	returnParameterTypes.push_back(TypeProvider::uint256());
	strings parameterNames;
	parameterNames.push_back("address");
	strings returnParameterNames;
	returnParameterNames.push_back("result");

	m_magicVariables.push_back(std::make_shared<MagicVariableDeclaration>(magicVariableToID("receivedVoteCount"), "receivedVoteCount", TypeProvider::function(
		parameterTypes,
		returnParameterTypes,
		parameterNames,
		returnParameterNames,
		FunctionType::Kind::ReceivedVoteCount,
		StateMutability::View,
		nullptr)
	));
}

void GlobalContext::addUsedVoteCountMethod() {
	// uint usedVoteCount(address)
	TypePointers parameterTypes;
	parameterTypes.push_back(TypeProvider::address());

	TypePointers returnParameterTypes;
	returnParameterTypes.push_back(TypeProvider::uint256());
	strings parameterNames;
	parameterNames.push_back("address");
	strings returnParameterNames;
	returnParameterNames.push_back("result");

	m_magicVariables.push_back(std::make_shared<MagicVariableDeclaration>(magicVariableToID("usedVoteCount"), "usedVoteCount", TypeProvider::function(
		parameterTypes,
		returnParameterTypes,
		parameterNames,
		returnParameterNames,
		FunctionType::Kind::UsedVoteCount,
		StateMutability::View,
		nullptr)
	));
}

void GlobalContext::setCurrentContract(ContractDefinition const& _contract)
{
	m_currentContract = &_contract;
}

std::vector<Declaration const*> GlobalContext::declarations() const
{
	std::vector<Declaration const*> declarations;
	declarations.reserve(m_magicVariables.size());
	for (ASTPointer<MagicVariableDeclaration const> const& variable: m_magicVariables)
		declarations.push_back(variable.get());
	return declarations;
}

MagicVariableDeclaration const* GlobalContext::currentThis() const
{
	if (!m_thisPointer[m_currentContract])
	{
		Type const* type = TypeProvider::emptyTuple();
		if (m_currentContract)
			type = TypeProvider::contract(*m_currentContract);
		m_thisPointer[m_currentContract] =
			std::make_shared<MagicVariableDeclaration>(magicVariableToID("this"), "this", type);
	}
	return m_thisPointer[m_currentContract].get();
}

MagicVariableDeclaration const* GlobalContext::currentSuper() const
{
	if (!m_superPointer[m_currentContract])
	{
		Type const* type = TypeProvider::emptyTuple();
		if (m_currentContract)
			type = TypeProvider::typeType(TypeProvider::contract(*m_currentContract, true));
		m_superPointer[m_currentContract] =
			std::make_shared<MagicVariableDeclaration>(magicVariableToID("super"), "super", type);
	}
	return m_superPointer[m_currentContract].get();
}

}
