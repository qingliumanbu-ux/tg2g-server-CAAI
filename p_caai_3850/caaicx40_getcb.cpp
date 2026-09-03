/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:
Version:    3.0
Date:		2018-06-21
Description:根据冶金规范、产线、规格全举出所有的产品的工艺路径、以及工艺路径对应到炼钢产线的坯料钢种、坯料规格；1、到各工序加工费取炼钢坯费用；2、判断是否过RH,如果过则还需取RH的费用；3、根据对应的炼钢产线的坯料钢种、规格取到炼钢坯的物料编码，再根据钢坯物料编码到bom信息表取对应的合金消耗，合金消耗再根据本月的价格算出合金成本；
4、轧钢及后部全部到各工序加工费取各个工艺路径的加工费； 
5、制造费用 = 炼钢费用（炼钢坯、RH精炼、合金）/成材率+轧钢费用+后部费用 ；
全成本 = 制造费用+期间费用 ；
含税全成本 = 全成本*1.13
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caaicx40_getcb)
//-EP_SYSTEM_HEAD_END
int f_caai_getcb(CString account_period, CString price_terms, CDbConnection * conn);
int f_caaicx40_getcb(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int fetchRowCount = 0;

	CString		account_period = " ";
	CString		price_terms = " ";
	CDecimal  amt = 0;
	CString   sub_backlog_code = "";
	CString   whole_backlog = "";
	CString   whole_backlog_desc = "";

	CModel tcaac05b("TCAAC05B");


	CString  sqlstr("");


	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_1(conn);

	try
	{
		//系统的分页类信息。
		CPageInfo pageInfo;

		tcaac05b.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		account_period = bcls_rec->Tables[0].Rows[0]["ACCOUNT_PERIOD"].ToString().SubstringNE(0, 6);
		price_terms = bcls_rec->Tables[0].Rows[0]["PRICE_TERMS"].ToString();

		sqlstr = " select CURRENT_STATUS from tcaac15"
			" where account_period=@account_period"
			" order by account_period desc  "
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			if (cmd_inq.GetString(1) == "4")
			{
				sprintf(s.msg, "会计期[%s]已经关账，不能获取，请重新选择会计期!", (const char*)account_period);

				throw CApplicationException(-1, s.msg, log.Location);
			}
		}
		cmd_inq.Close();


		doFlag = f_caai_getcb(account_period, price_terms, conn);
		if (doFlag < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}


	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应

		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	s.flag = doFlag;
	bcls_ret->SetSYS(s);
	return doFlag;
}